#include "texture.h"

solid_color::solid_color(const color& albedo) : albedo(albedo) {}

solid_color::solid_color(double red, double green, double blue) : solid_color(color(red, green, blue)) {}

color solid_color::value(double u, double v, const point3& p) const {
	return albedo;
}

// ================== image_texture ==================

image_texture::image_texture(const char* filename) : image(filename) {}

color image_texture::value(double u, double v, const point3& p) const {
    if (image.height() <= 0)
        return color(0, 1, 1);

    u = interval(0, 1).clamp(u);
    v = 1.0 - interval(0, 1).clamp(v);

    int i = int(u * image.width());
    int j = int(v * image.height());
    auto pixel = image.pixel_data(i, j);

    double color_scale = 1.0 / 255.0;
    return color(color_scale * pixel[0], color_scale * pixel[1], color_scale * pixel[2]);
}