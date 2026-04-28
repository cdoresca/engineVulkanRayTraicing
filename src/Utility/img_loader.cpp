#define _CRT_SECURE_NO_WARNINGS

#define STB_IMAGE_IMPLEMENTATION
#define STBI_FAILURE_USERMSG

#include "stb_image.h"
#include "img_loader.h"

#include <cstdlib>
#include <iostream>

// ------------------------------------------------------------
// Constructors / Destructor
// ------------------------------------------------------------
rtw_image::rtw_image() = default;

rtw_image::rtw_image(const char* image_filename) {
	auto filename = std::string(image_filename);
	auto imagedir = std::getenv("RTW_IMAGES");

	// Hunt for the image file in some likely locations.
	if (imagedir && load(std::string(imagedir) + "/" + image_filename)) return;
	if (load(filename)) return;
	if (load("images/" + filename)) return;
	if (load("../images/" + filename)) return;
	if (load("../../images/" + filename)) return;
	if (load("../../../images/" + filename)) return;
	if (load("../../../../images/" + filename)) return;
	if (load("../../../../../images/" + filename)) return;
	if (load("../../../../../../images/" + filename)) return;

	std::cerr << "ERROR: Could not load image file '" << image_filename << "'.\n";
}

rtw_image::~rtw_image() {
	delete[] bdata;
	STBI_FREE(fdata);
}

// ------------------------------------------------------------
// Public API
// ------------------------------------------------------------
bool rtw_image::load(const std::string& filename) {
	auto n = bytes_per_pixel; // Dummy out parameter
	fdata = stbi_loadf(
		filename.c_str(),
		&image_width,
		&image_height,
		&n,
		bytes_per_pixel
	);

	if (fdata == nullptr)
		return false;

	bytes_per_scanline = image_width * bytes_per_pixel;
	convert_to_bytes();
	return true;
}

int rtw_image::width() const {
	return (fdata == nullptr) ? 0 : image_width;
}

int rtw_image::height() const {
	return (fdata == nullptr) ? 0 : image_height;
}

const unsigned char* rtw_image::pixel_data(int x, int y) const {
	static unsigned char magenta[] = { 255, 0, 255 };

	if (bdata == nullptr)
		return magenta;

	x = clamp(x, 0, image_width);
	y = clamp(y, 0, image_height);

	return bdata + y * bytes_per_scanline + x * bytes_per_pixel;
}

// ------------------------------------------------------------
// Private helpers
// ------------------------------------------------------------
int rtw_image::clamp(int x, int low, int high) {
	if (x < low)  return low;
	if (x < high) return x;
	return high - 1;
}

unsigned char rtw_image::float_to_byte(float value) {
	if (value <= 0.0f) return 0;
	if (value >= 1.0f) return 255;
	return static_cast<unsigned char>(256.0f * value);
}

void rtw_image::convert_to_bytes() {
	int total_bytes = image_width * image_height * bytes_per_pixel;
	bdata = new unsigned char[total_bytes];

	auto* bptr = bdata;
	auto* fptr = fdata;

	for (int i = 0; i < total_bytes; ++i, ++fptr, ++bptr)
		*bptr = float_to_byte(*fptr);
}
