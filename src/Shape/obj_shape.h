#pragma once

#include "mesh.h"
#include "triangle.h"
#include "shape_list.h"
#include "Material/material.h"

#include <embree4/rtcore.h>

// Store per-triangle data for hit records
// obj_shape.h
struct tri_data {
	std::shared_ptr<material> mat;
	point3 v0, v1, v2;
	vec3   normal;
	glm::vec2 uv0, uv1, uv2;
};

class obj_shape : public shape {
public:
	obj_shape(
		const point3& center,
		const std::vector<mesh>& meshes,
		const std::unordered_map<std::string, std::shared_ptr<material>>& mat_map,
		std::shared_ptr<material> fallback,
		shape_list& lights
	);

	~obj_shape();

	bool hit(const ray& r, interval ray_t, hit_record& rec) const override;
	double pdf_value(const point3& origin, const vec3& direction) const override;
	point3 get_center() const override;

private:
	void commit_scene();

	RTCDevice device;
	RTCScene  embree_scene;

	std::vector<tri_data> triangles;
	point3 center;
};