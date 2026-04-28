#include "utility/vec3.h"

class sampler {
public:
	int    sqrt_spp;				// Square root of number of samples per pixel
	int    samples_per_pixel = 1;   // Count of random samples for each pixel
	double recip_sqrt_spp;			// 1 / sqrt_spp
	double pixel_samples_scale;

	sampler(int s);
	virtual ~sampler() = default;

	virtual vec3 sample_square(int s_i, int s_j) const = 0;
};

class stratified : public sampler {
public:
	stratified(int s);

	vec3 sample_square(int s_i, int s_j) const override;
};

class uniform : public sampler {
public:
	uniform(int s);

	vec3 sample_square(int s_i, int s_j) const override;
};

class sobol : public sampler {
public:
	sobol(int s);

	vec3 sample_square(int s_i, int s_j) const override;

private:
	static double sobol_sample(uint32_t index, int dimension);
};