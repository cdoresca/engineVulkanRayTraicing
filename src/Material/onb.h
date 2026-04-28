#pragma once
#include "Utility/vec3.h"

class onb {

	vec3 axis[3];

	public :

		onb(const vec3& v);

		const vec3& u() const;
		const vec3& v() const;
		const vec3& w() const;

		vec3 transform(const vec3& v) const;
};