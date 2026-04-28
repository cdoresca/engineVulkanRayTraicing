#pragma once

#include "utility/vec3.h"
#include "utility/ray.h"
#include "utility/interval.h"
#include <glm/glm.hpp>

class material;

class hit_record {
public:
	point3 p;
	vec3 normal;
	shared_ptr<material> mat;
	double t;
	double u;
	double v;
	bool front_face;
	
	void set_face_normal(const ray& r, const vec3& outward_normal);
};

class shape {
public:
	virtual ~shape() = default;
	virtual bool hit(const ray& r, interval ray_t, hit_record& rec) const = 0;
	virtual bool emits() const { return false; }

	virtual point3 get_center() const {
		return point3(0, 0, 0);
	}

	virtual double pdf_value(const point3& origin, const vec3& direction) const { return 0.0; }
	virtual vec3 random(const point3& origin) const { return vec3(1, 0, 0); }

	glm::mat4 transform;
	uint32_t index;
};
