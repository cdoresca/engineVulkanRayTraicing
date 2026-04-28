#pragma once

#include "utility/ray.h"
#include "utility/color.h"

class world;

class tracer {
public:
	virtual ~tracer() = default;

	virtual color cast_ray(const ray& r, int depth, const world& w, bool count_emission = true) const = 0;
};

class path_tracing : public tracer {
public:
	color cast_ray(const ray& r, int depth, const world& w, bool count_emission = true) const override;
	color cast_ray_no_emit(const ray& r, int depth, const world& w) const;
};

class ray_tracing : public tracer {
public:
	color cast_ray(const ray& r, int depth, const world& w, bool count_emission = true) const override;
};

