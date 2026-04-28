#pragma once

#include "utility/vec3.h"
#include "utility/ray.h"
#include "utility/color.h"
#include "shape/shape.h"
#include "material/material.h"

#include <iostream>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/matrix_clip_space.hpp>


const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

class world;

class camera
{
public:
	double aspect_ratio = 1.0; // Ratio of image width over height
	int image_width = 100;	   // Rendered image width in pixel count
	int max_depth = 10;		   // Maximum number of ray bounces into scene

	double vfov = 90;				   // Vertical view angle (field of view)
	point3 lookfrom = point3(0, 0, 0); // Point camera is looking from
	point3 lookat = point3(0, 0, -1);  // Point camera is looking at
	vec3 vup = vec3(0, 1, 0);		   // Camera-relative "up" direction

	double defocus_angle = 0; // Variation angle of rays through each pixel
	double focus_dist = 10;	  // Distance from camera lookfrom point to plane of perfect focus

	 char filename[48];

	void render(const world &w, std::atomic<int> *rows_done);

private:
	int image_height;	// Rendered image height
	point3 center;		// Camera center
	point3 pixel00_loc; // Location of pixel 0, 0
	vec3 pixel_delta_u; // Offset to pixel to the right
	vec3 pixel_delta_v; // Offset to pixel below
	vec3 u, v, w;		// Camera frame basis vectors

	vec3 defocus_disk_u; // Defocus disk horizontal radius
	vec3 defocus_disk_v; // Defocus disk vertical radius

	void initialize();
	ray get_ray(const world &w, int i, int j, int s_i, int s_j) const;
	point3 defocus_disk_sample() const;
};

/**
 * @brief 
 * Le struct Camera représente les information utiles pour créer un espace orthonomé
 */
struct Camera
{
	glm::vec4 position;
	glm::vec4 lookAt;
	glm::vec4 up;
	float fov;
	uint32_t frame;
	uint32_t shader = 0;
};


/*
	fonction utilitaire pour donner a imguizmo
*/
glm::mat4 getViewMatrix(const Camera& cam);

/*
	fonction utilitaire pour donner a imguizmo
*/
glm::mat4 getProjectionMatrix(const Camera& cam);

