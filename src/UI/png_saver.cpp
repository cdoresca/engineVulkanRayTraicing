#define _CRT_SECURE_NO_WARNINGS
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#include "Utility/color.h"
#include "png_saver.h"
#include <vector>



bool png_saver::save_image(int width, int height, std::vector<std::vector<color>> buffers, char* name) {
    int channels = 4; // RGBA format (We're just gonna add the a)
    std::vector<uint8_t> pixels; //(width* height* channels)

    for (const std::vector<color>& buf : buffers) {
		for (const color& col : buf) {
            auto r = col.x();
            auto g = col.y();
            auto b = col.z();

            // Replace NaN components with zero.
            if (r != r) r = 0.0;
            if (g != g) g = 0.0;
            if (b != b) b = 0.0;

            // Apply a linear to gamma transform for gamma 2
            r = linear_to_gamma(r);
            g = linear_to_gamma(g);
            b = linear_to_gamma(b);

            // Translate the [0,1] component values to the byte range [0,255].
            static const interval intensity(0.000, 0.999);
            int rbyte = int(256 * intensity.clamp(r));
            int gbyte = int(256 * intensity.clamp(g));
            int bbyte = int(256 * intensity.clamp(b));

			pixels.push_back(static_cast<uint8_t>(rbyte)); //R
            pixels.push_back(static_cast<uint8_t>(gbyte)); //G
            pixels.push_back(static_cast<uint8_t>(bbyte)); //B
			pixels.push_back(static_cast<uint8_t>(255)); //A (fully opaque)
        }
    }

    const char* ext = ".png";
    const char* folder = "output/";
    char* filename = new char[strlen(folder) + strlen(name) + strlen(ext) + 1];
    strcpy(filename, folder);
    strcat(filename, name);
    strcat(filename, ext);

    int stride_bytes = width * channels; //Distance in bytes from start of one row to the next

    // Write the PNG file using stb
    if (stbi_write_png(filename, width, height, channels, pixels.data(), stride_bytes)) {
		return true;
    }
    else {
		return false;
    }
}
