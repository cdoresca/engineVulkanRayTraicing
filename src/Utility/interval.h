#pragma once

class interval {
public:
	double min, max;

	// Constructors
	interval();
	interval(double min, double max);
	interval(const interval& a, const interval& b);

	// Methods
	double size() const;
	double clamp(double x) const;
	bool contains(double x) const;
	bool surrounds(double x) const;
	interval expand(double delta) const;

	// Predefined intervals
	static const interval empty;
	static const interval universe;
};

//overrides
interval operator+(const interval& ival, double displacement);
interval operator+(double displacement, const interval& ival);