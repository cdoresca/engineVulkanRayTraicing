#pragma once

#include "vec3.h"

class ray {
public:
	// Constructors
	ray();
	ray(const point3& origin, const vec3& direction);

	// Accessors
	const point3& origin() const;
	const vec3& direction() const;

	// Methods
	point3 at(double t) const;

private:
	point3 orig;
	vec3 dir;
};
