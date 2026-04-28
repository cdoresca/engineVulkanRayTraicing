#pragma once

#include "vec3.h"
#include "interval.h"
#include <vector>

// Color is just an alias for vec3
using color = vec3;

double linear_to_gamma(double linear_component);
color hex_to_color(const std::string& hex);

inline const color RED = color(0.65, 0.05, 0.05);
inline const color BLUE = color(0.10, 0.25, 0.75);
inline const color GREEN = color(0.12, 0.45, 0.15);
inline const color BLACK = color(0.00, 0.00, 0.00);
inline const color WHITE = color(0.73, 0.73, 0.73);
inline const color YELLOW = color(0.60, 0.60, 0.00);
inline const color SKY_BLUE = color(0.70, 0.80, 1.00);