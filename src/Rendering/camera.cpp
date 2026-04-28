#include "camera.h"
#include "world.h"
#include "tracer.h"
#include "Material/pdf.h"
#include "ui/png_saver.h"

#include <thread>
#include <vector>
#include <sstream>
#include <iostream>
#include <mutex>
#include <fstream>
#include <iomanip>

void camera::render(const world& w, std::atomic<int>* rows_done) {
	initialize();

	const int num_threads = std::thread::hardware_concurrency() - 1;

	std::vector<std::vector<color>> buffers(num_threads);

	std::atomic<int> rows_done_atomic = { 0 };
	std::atomic<bool> render_complete = { false };

	// Worker function
	auto render_rows = [&](int thread_id, int start_row, int end_row) {
		for (int j = start_row; j < end_row; ++j) {
			for (int i = 0; i < image_width; ++i) {
				color pixel_color(0, 0, 0);

				for (int s_j = 0; s_j < w.sampler_type->sqrt_spp; s_j++) {
					for (int s_i = 0; s_i < w.sampler_type->sqrt_spp; s_i++) {
						ray r = get_ray(w, i, j, s_i, s_j);
						pixel_color += w.tracer_type->cast_ray(r, max_depth, w);
					}
				}

				buffers[thread_id].push_back(w.sampler_type->pixel_samples_scale * pixel_color);
			}
			++rows_done_atomic;
		}
	};

	// Progress reporter
	std::thread progress_thread([&]() {
		using clock = std::chrono::steady_clock;

		const int    bar_width = 40;
		const double total = static_cast<double>(image_height);
		const auto   start_time = clock::now();

		while (!render_complete.load()) {
			int    done = rows_done_atomic.load();
			double progress = done / total;

			// ETA
			auto   elapsed_s = std::chrono::duration<double>(clock::now() - start_time).count();
			double eta_s = (done > 0) ? (elapsed_s / done) * (image_height - done) : 0.0;
			int    eta_min = static_cast<int>(eta_s) / 60;
			int    eta_sec = static_cast<int>(eta_s) % 60;

			// Build bar
			int filled = static_cast<int>(progress * bar_width);
			std::string bar(filled, '#');
			if (filled < bar_width) {
				bar += std::string(bar_width - filled - 1, ' ');
			}

			std::clog << "\r[" << bar << "] "
				<< std::setw(3) << static_cast<int>(progress * 100) << "% "
				<< "| " << std::setw(4) << done << "/" << image_height << " rows "
				<< "| ETA: " << std::setw(2) << std::setfill('0') << eta_min
				<< ":" << std::setw(2) << std::setfill('0') << eta_sec
				<< std::setfill(' ')
				<< std::flush;

			rows_done->store(image_height - done);
			std::this_thread::sleep_for(std::chrono::milliseconds(1000));
		}

		// Final flush
		std::string blank(bar_width + 60, ' ');
		std::clog << "\r" << blank << "\rDone.\n" << std::flush;
		});

	// Launch worker threads
	std::vector<std::thread> threads;
	threads.reserve(num_threads);

	int rows_per_thread = image_height / num_threads;
	int remaining_rows = image_height % num_threads;
	int current_row = 0;

	for (int t = 0; t < num_threads; ++t) {
		int start_row = current_row;
		int end_row = start_row + rows_per_thread + (t < remaining_rows ? 1 : 0);

		threads.emplace_back(render_rows, t, start_row, end_row);
		current_row = end_row;
	}

	// Wait for workers, then signal progress thread
	for (auto& th : threads)
		th.join();

	render_complete.store(true);
	progress_thread.join();

	png_saver::save_image(image_width, image_height, buffers, filename);
}

void camera::initialize() {
	image_height = int(image_width / aspect_ratio);
	image_height = (image_height < 1) ? 1 : image_height;

	center = lookfrom;

	// Determine viewport dimensions
	auto theta = degrees_to_radians(vfov);
	auto h = std::tan(theta / 2);
	auto viewport_height = 2 * h * focus_dist;
	auto viewport_width = viewport_height * (double(image_width) / image_height);

	// Calculate the u,v,w unit basis vectors for the camera coordinate frame.
	w = unit_vector(lookfrom - lookat);
	u = unit_vector(cross(vup, w));
	v = cross(w, u);

	// Calculate the vectors across the horizontal and down the vertical viewport edges.
	vec3 viewport_u = viewport_width * u;    // Vector across viewport horizontal edge
	vec3 viewport_v = viewport_height * -v;  // Vector down viewport vertical edge

	// Calculate the horizontal and vertical delta vectors from pixel to pixel.
	pixel_delta_u = viewport_u / image_width;
	pixel_delta_v = viewport_v / image_height;

	// Calculate the location of the upper left pixel.
	auto viewport_upper_left = center - (focus_dist * w) - viewport_u / 2 - viewport_v / 2;
	pixel00_loc = viewport_upper_left + 0.5 * (pixel_delta_u + pixel_delta_v);

	// Calculate the camera defocus disk basis vectors.
	auto defocus_radius = focus_dist * std::tan(degrees_to_radians(defocus_angle / 2));
	defocus_disk_u = u * defocus_radius;
	defocus_disk_v = v * defocus_radius;
}

point3 camera::defocus_disk_sample() const {
	// Returns a random point in the camera defocus disk.
	auto p = random_in_unit_disk();
	return center + (p[0] * defocus_disk_u) + (p[1] * defocus_disk_v);
}

ray camera::get_ray(const world& w, int i, int j, int s_i, int s_j) const {
	// Construct a camera ray originating from the defocus disk and directed at a randomly
	// sampled point around the pixel location i, j.

	auto offset = w.sampler_type->sample_square(s_i, s_j);
	auto pixel_sample = pixel00_loc
		+ ((i + offset.x()) * pixel_delta_u)
		+ ((j + offset.y()) * pixel_delta_v);

	auto ray_origin = (defocus_angle <= 0) ? center : defocus_disk_sample();
	auto ray_direction = pixel_sample - ray_origin;

	return ray(ray_origin, ray_direction);
}

glm::mat4 getViewMatrix(const Camera& cam)
{
	return glm::lookAt(
		glm::vec3(cam.position), // eye
		glm::vec3(cam.lookAt),	 // center
		glm::vec3(cam.up)		 // up
	);

}

glm::mat4 getProjectionMatrix(const Camera& cam)
{
	glm::mat4 proj = glm::perspective(
		glm::radians(cam.fov),
		float(WIDTH) / float(HEIGHT),
		0.1f,
		100000.0f);
	//proj[1][1] *= -1.0f;
	return proj;
}

