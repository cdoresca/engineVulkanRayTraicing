#include "interval.h"

#include <limits>

// Convenient infinity constant
static constexpr double infinity = std::numeric_limits<double>::infinity();

// Constructors
interval::interval() : min(+infinity), max(-infinity) {} // Default interval is empty

interval::interval(double min, double max) : min(min), max(max) {}

interval::interval(const interval& a, const interval& b) {
	// Create the interval tightly enclosing the two input intervals.
	min = a.min <= b.min ? a.min : b.min;
	max = a.max >= b.max ? a.max : b.max;
}

// Methods
double interval::size() const {
	return max - min;
}

double interval::clamp(double x) const {
	if (x < min) return min;
	if (x > max) return max;
	return x;
}

bool interval::contains(double x) const {
	return min <= x && x <= max;
}

bool interval::surrounds(double x) const {
	return min < x && x < max;
}

interval interval::expand(double delta) const {
	auto padding = delta / 2;
	return interval(min - padding, max + padding);
}

// Static members
const interval interval::empty(+infinity, -infinity);
const interval interval::universe(-infinity, +infinity);


//overrides
interval operator+(const interval& ival, double displacement) {
	return interval(ival.min + displacement, ival.max + displacement);
}

interval operator+(double displacement, const interval& ival) {
	return ival + displacement;
}
