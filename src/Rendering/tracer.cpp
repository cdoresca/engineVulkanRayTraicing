#include "tracer.h"
#include "world.h"
#include "shape/shape.h"
#include "utility/color.h"
#include "Material/material.h"

color path_tracing::cast_ray(const ray& r, int depth, const world& w, bool count_emission) const {
	if (depth <= 0)
		return color(0, 0, 0);

	hit_record rec;
	if (!w.scene.hit(r, interval(0.001, infinity), rec))
		return w.background;

	scatter_record srec;
	color color_from_emission(0, 0, 0);
	if (count_emission)
		color_from_emission = rec.mat->emitted(r, rec, rec.u, rec.v, rec.p);

	if (!rec.mat->scatter(r, rec, srec))
		return color_from_emission;

	if (srec.skip_pdf)
		return srec.attenuation * cast_ray(srec.skip_pdf_ray, depth - 1, w, true);

	// ---- Shading frame ------------------------------------------------
	// Prefer tangent stored in hit_record if available (preserves mesh
	// anisotropy). Fall back to computing one from the normal.
	vec3 n = rec.normal;
	vec3 wo = unit_vector(-r.direction());
	vec3 t, b;

#ifdef REC_HAS_TANGENT
	t = rec.tangent;
	b = cross(n, t);
#else
	if (std::fabs(n.x()) > std::fabs(n.z()))
		t = unit_vector(vec3(-n.y(), n.x(), 0.0));
	else
		t = unit_vector(vec3(0.0, -n.z(), n.y()));
	b = cross(n, t);
#endif

	// ---- Material helpers ---------------------------------------------
	const disney_principled* disney_mat =
		dynamic_cast<const disney_principled*>(rec.mat.get());
	color base_color(1, 1, 1);
	if (disney_mat)
		base_color = disney_mat->tex->value(rec.u, rec.v, rec.p);

	auto eval_brdf = [&](const vec3& wi) -> color {
		if (disney_mat)
			return disney_mat->evaluate_brdf(n, t, b, base_color, wo, wi);
		return color(1, 1, 1);
		};

	auto luminance = [](const color& c) -> double {
		return 0.2126 * c.x() + 0.7152 * c.y() + 0.0722 * c.z();
		};

	auto power_heuristic = [](double p_a, double p_b) -> double {
		double a = p_a * p_a, b = p_b * p_b;
		return (a + b) > 0.0 ? a / (a + b) : 0.0;
		};

	auto smoothstep = [](double lo, double hi, double x) -> double {
		double t = std::clamp((x - lo) / (hi - lo), 0.0, 1.0);
		return t * t * (3.0 - 2.0 * t);
		};

	disney_mixture_pdf* disney_pdf =
		dynamic_cast<disney_mixture_pdf*>(srec.pdf_ptr.get());

	// ---- Sample the indirect direction ONCE ---------------------------
	// We do this before the NEE block so that:
	//   (a) last_lobe is set and can drive nee_weight,
	//   (b) the same wi is reused for the BRDF-MIS term inside NEE,
	//       avoiding a redundant generate() call that would also clobber
	//       last_lobe.
	vec3  indirect_wi_dir = srec.pdf_ptr->generate();
	vec3  indirect_wi = unit_vector(indirect_wi_dir);
	double indirect_p_brdf = srec.pdf_ptr->value(indirect_wi_dir);
	double indirect_NoL = std::max(0.0, dot(n, indirect_wi));

	// ---- NEE weight from last_lobe ------------------------------------
	// Read last_lobe AFTER generate() and BEFORE any further generate()
	// calls so the value is stable.
	double nee_weight = 1.0;
	if (disney_pdf) {
		if (disney_pdf->last_lobe == DisneyLobe::Specular) {
			double eff_roughness = disney_mat->roughness * (1.0 - 0.9 * disney_mat->metallic);
			nee_weight = smoothstep(0.05, 0.25, eff_roughness);
		}
		// Diffuse and Clearcoat keep nee_weight = 1.0
	}

	bool use_nee = (nee_weight >= 1.0) || (random_double() < nee_weight);

	// ---- DIRECT lighting (NEE + MIS) ----------------------------------
	color direct(0, 0, 0);
	if (use_nee && !w.lights.empty()) {
		auto light_pdf = make_shared<shape_pdf>(w.lights, rec.p);

		// --- MIS term 1: light-sampled direction ---
		{
			vec3 wi_dir = light_pdf->generate();
			vec3 wi = unit_vector(wi_dir);
			double p_light = light_pdf->value(wi_dir);
			double p_brdf = srec.pdf_ptr->value(wi_dir);
			double NoL = std::max(0.0, dot(n, wi));

			if (p_light > 1e-4 && NoL > 0.0) {
				ray shadow_ray(rec.p, wi_dir);
				hit_record light_rec;
				color Le(0, 0, 0);
				if (w.scene.hit(shadow_ray, interval(0.001, infinity), light_rec))
					Le = light_rec.mat->emitted(shadow_ray, light_rec,
						light_rec.u, light_rec.v, light_rec.p);
				if (luminance(Le) > 0.0) {
					double w_light = power_heuristic(p_light, p_brdf);
					direct += eval_brdf(wi) * NoL * w_light * Le / p_light;
				}
			}
		}

		// --- MIS term 2: BRDF-sampled direction (reuse indirect_wi) ---
		{
			double p_brdf = indirect_p_brdf;
			double p_light = light_pdf->value(indirect_wi_dir);
			double NoL = indirect_NoL;

			if (p_brdf > 1e-4 && NoL > 0.0) {
				ray brdf_ray(rec.p, indirect_wi_dir);
				hit_record light_rec;
				color Le(0, 0, 0);
				if (w.scene.hit(brdf_ray, interval(0.001, infinity), light_rec))
					Le = light_rec.mat->emitted(brdf_ray, light_rec,
						light_rec.u, light_rec.v, light_rec.p);
				if (luminance(Le) > 0.0) {
					double w_brdf = power_heuristic(p_brdf, p_light);
					direct += eval_brdf(indirect_wi) * NoL * w_brdf * Le / p_brdf;
				}
			}
		}

		direct /= nee_weight;
	}

	// ---- INDIRECT lighting --------------------------------------------
	// count_emission is false whenever NEE ran this bounce: NEE's MIS
	// terms already account for emitter hits via the BRDF-sample term.
	// Only re-enable emission when NEE was skipped (e.g. mirror specular)
	// so that light sources remain visible in perfect reflections.
	color indirect(0, 0, 0);
	if (indirect_p_brdf > 1e-4 && indirect_NoL > 0.0) {
		bool indirect_counts_emission = !use_nee;
		color Li = cast_ray(ray(rec.p, indirect_wi_dir), depth - 1, w, indirect_counts_emission);
		indirect = eval_brdf(indirect_wi) * indirect_NoL * Li / indirect_p_brdf;
	}

	return color_from_emission + direct + indirect;
}


color ray_tracing::cast_ray(const ray& r, int depth, const world& w, bool count_emission) const {
	if (depth <= 0)
		return color(0, 0, 0);

	hit_record rec;
	if (!w.scene.hit(r, interval(0.001, infinity), rec))
		return w.background;

	// Emission — suppressed on indirect hits to match NEE in path tracer
	color Le(0, 0, 0);
	if (count_emission)
		Le = rec.mat->emitted(r, rec, rec.u, rec.v, rec.p);

	scatter_record srec;
	if (!rec.mat->scatter(r, rec, srec))
		return Le; // hit a light or non-scattering surface

	// Specular / glass — recurse directly, no direct lighting
	if (srec.skip_pdf)
		return Le + srec.attenuation * cast_ray(srec.skip_pdf_ray, depth - 1, w, true);

	// Shared geometry
	vec3 n = rec.normal;
	vec3 wo = unit_vector(-r.direction());

	vec3 t, b;
	if (std::fabs(n.x()) > std::fabs(n.z()))
		t = unit_vector(vec3(-n.y(), n.x(), 0.0));
	else
		t = unit_vector(vec3(0.0, -n.z(), n.y()));
	b = cross(n, t);

	const disney_principled* disney_mat =
		dynamic_cast<const disney_principled*>(rec.mat.get());
	color base_color(1, 1, 1);
	if (disney_mat)
		base_color = disney_mat->tex->value(rec.u, rec.v, rec.p);

	auto eval_brdf = [&](const vec3& wi) -> color {
		if (disney_mat)
			return disney_mat->evaluate_brdf(n, t, b, base_color, wo, wi);
		return srec.attenuation * (1.0 / pi);
		};

	// Direct lighting — one shadow ray per light
	color L_direct(0, 0, 0);
	if (!w.lights.empty()) {
		for (const auto& light_obj : w.lights.objects) {
			vec3 to_light = light_obj->random(rec.p);
			double dist = to_light.length();
			if (dist <= 0.0) continue;

			vec3 wi = unit_vector(to_light);
			double NoL = std::max(0.0, dot(n, wi));
			if (NoL <= 0.0) continue;

			// Shadow ray — check for occlusion up to the light
			ray shadow_ray(rec.p, wi);
			hit_record lrec;
			color Le_light(0, 0, 0);

			if (w.scene.hit(shadow_ray, interval(0.001, dist + 0.001), lrec))
				Le_light = lrec.mat->emitted(shadow_ray, lrec, lrec.u, lrec.v, lrec.p);

			if (Le_light.x() == 0 && Le_light.y() == 0 && Le_light.z() == 0) continue;

			// Solid-angle PDF for this light sample
			double pdf = light_obj->pdf_value(rec.p, wi);
			if (pdf <= 1e-4) continue;

			L_direct += eval_brdf(wi) * NoL * Le_light / pdf;
		}
	}
	else {
		vec3 ao_dir = random_cosine_direction();
		vec3 wi = unit_vector(ao_dir.x() * t + ao_dir.y() * b + ao_dir.z() * n);
		double NoL = std::max(0.0, dot(n, wi));
		L_direct = eval_brdf(wi) * w.background * NoL;
	}

	// Ambient — classic Whitted constant term to approximate indirect light
	color L_ambient(0, 0, 0);
	if (L_direct == BLACK)
	{
		const int    ao_samples = min(10, w.sampler_type->samples_per_pixel);
		const double ao_intensity = 1;

		for (int i = 0; i < ao_samples; ++i) {
			vec3 ao_dir = random_cosine_direction();
			vec3 ao_wi = unit_vector(ao_dir.x() * t + ao_dir.y() * b + ao_dir.z() * n);
			if (dot(ao_wi, n) <= 0.0) continue;

			double NoL = std::max(0.0, dot(n, ao_wi));
			ray ao_ray(rec.p, ao_wi);
			hit_record ao_rec;

			if (!w.scene.hit(ao_ray, interval(0.001, infinity), ao_rec)) {
				// Unoccluded — pick up sky radiance directly
				L_ambient += base_color * w.background * NoL;
			}
			else {
				// Occluded — one bounce: sample lighting at the hit point
				color Le_bounce = ao_rec.mat->emitted(ao_ray, ao_rec, ao_rec.u, ao_rec.v, ao_rec.p);

				// Gather direct light at the bounce point
				color L_bounce_direct(0, 0, 0);
				vec3 bn = ao_rec.normal;

				if (!w.lights.empty()) {
					for (const auto& light_obj : w.lights.objects) {
						vec3 to_light = light_obj->random(ao_rec.p);
						double dist = to_light.length();
						if (dist <= 0.0) continue;

						vec3 wi = unit_vector(to_light);
						double bNoL = std::max(0.0, dot(bn, wi));
						if (bNoL <= 0.0) continue;

						ray shadow_ray(ao_rec.p, wi);
						hit_record srec2;
						color Le_light(0, 0, 0);
						if (w.scene.hit(shadow_ray, interval(0.001, dist + 0.001), srec2))
							Le_light = srec2.mat->emitted(shadow_ray, srec2, srec2.u, srec2.v, srec2.p);
						if (Le_light.x() == 0 && Le_light.y() == 0 && Le_light.z() == 0) continue;

						double pdf = light_obj->pdf_value(ao_rec.p, wi);
						if (pdf <= 1e-4) continue;

						// Approximate bounce BRDF as Lambertian using the hit albedo
						color bounce_albedo = ao_rec.mat->emitted(ao_ray, ao_rec, ao_rec.u, ao_rec.v, ao_rec.p);
						scatter_record bounce_srec;
						if (ao_rec.mat->scatter(ao_ray, ao_rec, bounce_srec))
							bounce_albedo = bounce_srec.attenuation;

						L_bounce_direct += (bounce_albedo * (1.0 / pi)) * bNoL * Le_light / pdf;
					}
				}
				// Attenuate through the primary surface's albedo
				L_ambient += base_color * (Le_bounce + L_bounce_direct) * NoL * 0.5;
			}
		}
		if (ao_samples > 0)
			L_ambient = L_ambient * (ao_intensity / ao_samples);
	}

	return Le + L_direct + L_ambient;
}