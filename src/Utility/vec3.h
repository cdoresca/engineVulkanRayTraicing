#pragma once

#include "util.h"
#include <ostream>
#include <glm/glm.hpp>

class vec3 {
public:
	double e[3];

	// Constructors
	vec3();
	vec3(double e0, double e1, double e2);

	// Accessors
	double x() const;
	double y() const;
	double z() const;

	// Operators
	vec3 operator-() const;
	double operator[](int i) const;
	double& operator[](int i);

	vec3& operator+=(const vec3& v);
	bool  operator==(const vec3& v);
	vec3& operator*=(double t);
	vec3& operator/=(double t);

	// Length
	double length() const;
	double length_squared() const;
	bool near_zero() const;

	double max() const {
		return std::max(x(), std::max(y(), z()));
	}

	static vec3 random();
	static vec3 random(double min, double max);

	double* data();
};

// point3 is just an alias for vec3, but useful for geometric clarity
using point3 = vec3;

// ------------------------------------------------------------
// Vector utility function declarations

std::ostream& operator<<(std::ostream& out, const vec3& v);

vec3 operator+(const vec3& u, const vec3& v);
vec3 operator+(const vec3& u, const glm::vec3& v);
vec3 operator-(const vec3& u, const vec3& v);
vec3 operator*(const vec3& u, const vec3& v);
vec3 operator*(double t, const vec3& v);
vec3 operator*(const vec3& v, double t);
vec3 operator/(const vec3& v, double t);

double dot(const vec3& u, const vec3& v);
vec3 cross(const vec3& u, const vec3& v);
vec3 unit_vector(const vec3& v);
vec3 random_in_unit_disk();

vec3 random_unit_vector();
vec3 random_on_hemisphere(const vec3& normal);
vec3 random_cosine_direction();

vec3 reflect(const vec3& v, const vec3& n);
vec3 refract(const vec3& uv, const vec3& n, double etai_over_etat);