#include "color.h"

#include <cmath>
#include <string>

double linear_to_gamma(double linear_component) {
	if (linear_component > 0)
		return std::sqrt(linear_component);

	return 0;
}

color hex_to_color(const std::string& hex) {

	std::string hexcode = hex;
	// Remove leading '#' if present
	if (!hexcode.empty() && hexcode[0] == '#')
		hexcode.erase(0, 1);

	// Return black if hexcode is not 6 char long
	if (hexcode.length() != 6)
		return BLACK;

	int r = std::stoi(hexcode.substr(0, 2), nullptr, 16);
	int g = std::stoi(hexcode.substr(2, 2), nullptr, 16);
	int b = std::stoi(hexcode.substr(4, 2), nullptr, 16);

	return vec3(
		r / 255.0f,
		g / 255.0f,
		b / 255.0f
	);
}
