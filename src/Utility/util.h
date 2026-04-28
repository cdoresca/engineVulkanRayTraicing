#pragma once
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <memory>


// C++ Std Usings

using std::make_shared;
using std::shared_ptr;


// Constants

const double infinity = std::numeric_limits<double>::infinity();
const double pi = 3.1415926535897932385;
const double EPSILON = 1e-4;
static const uint32_t SOBOL_DIRS[2][32] = {
	// Dimension 0
	{
		0x80000000u, 0x40000000u, 0x20000000u, 0x10000000u,
		0x08000000u, 0x04000000u, 0x02000000u, 0x01000000u,
		0x00800000u, 0x00400000u, 0x00200000u, 0x00100000u,
		0x00080000u, 0x00040000u, 0x00020000u, 0x00010000u,
		0x00008000u, 0x00004000u, 0x00002000u, 0x00001000u,
		0x00000800u, 0x00000400u, 0x00000200u, 0x00000100u,
		0x00000080u, 0x00000040u, 0x00000020u, 0x00000010u,
		0x00000008u, 0x00000004u, 0x00000002u, 0x00000001u
	},
	// Dimension 1 (primitive polynomial x^3 + x + 1)
	{
		0x80000000u, 0xC0000000u, 0x60000000u, 0x30000000u,
		0x18000000u, 0x0C000000u, 0x06000000u, 0x03000000u,
		0x01800000u, 0x00C00000u, 0x00600000u, 0x00300000u,
		0x00180000u, 0x000C0000u, 0x00060000u, 0x00030000u,
		0x00018000u, 0x0000C000u, 0x00006000u, 0x00003000u,
		0x00001800u, 0x00000C00u, 0x00000600u, 0x00000300u,
		0x00000180u, 0x000000C0u, 0x00000060u, 0x00000030u,
		0x00000018u, 0x0000000Cu, 0x00000006u, 0x00000003u
	}
};

// Utility Functions

inline double degrees_to_radians(double degrees) {
	return degrees * pi / 180.0;
}

inline double random_double() {
	// Returns a random real in [0,1).
	return std::rand() / (RAND_MAX + 1.0);
}

inline double random_double(double min, double max) {
	// Returns a random real in [min,max).
	return min + (max - min) * random_double();
}

inline int random_int(int min, int max) {
	// Returns a random integer in [min,max].
	return int(random_double(min, max + 1));
}