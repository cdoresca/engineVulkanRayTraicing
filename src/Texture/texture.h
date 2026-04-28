#pragma once

#include "utility/vec3.h"
#include "utility/color.h"
#include "utility/img_loader.h"

class texture {
public:
	virtual ~texture() = default;

	virtual color value(double u, double v, const point3& p) const = 0;
};

class solid_color : public texture {
public:
	solid_color(const color& albedo);
	solid_color(double red, double green, double blue);

	color value(double u, double v, const point3& p) const override;

private:
	color albedo;
};

class image_texture : public texture {
public:
	explicit image_texture(const char* filename);

	color value(double u, double v, const point3& p) const override;

private:
	rtw_image image;
};