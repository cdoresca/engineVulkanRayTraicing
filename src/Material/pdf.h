#pragma once
#include "onb.h"
#include "utility/color.h"
#include "utility/vec3.h"
#include "Shape/shape.h"

#include <algorithm>
#include <cmath>
#include <memory>

// ----------------------- Helper Functions ----------------------------
static inline double saturate(double x) { return std::clamp(x, 0.0, 1.0); }
static inline double sqr(double x) { return x * x; }
static inline double max3(double a, double b, double c) { return std::max({ a,b,c }); }

static inline double luminance(const color& c) {
    return 0.2126 * c.x() + 0.7152 * c.y() + 0.0722 * c.z();
}

static inline color fresnel_schlick(const color& f0, double cosx) {
    double m = saturate(1.0 - cosx);
    double m2 = sqr(m);
    double m5 = sqr(m2) * m;

    return f0 + (color(1.0, 1.0, 1.0) - f0) * m5;
}

static inline double cos_theta(const vec3& n, const vec3& v) {
    return std::clamp(dot(n, v), -1.0, 1.0);
}

static inline double D_GTR2_aniso(const vec3& h, const vec3& n, const vec3& t, const vec3& b, double ax, double ay) {
    double hx = dot(h, t), hy = dot(h, b), hn = dot(h, n);
    double denom = (sqr(hx / ax) + sqr(hy / ay) + sqr(hn));

    return 1.0 / (pi * ax * ay * sqr(denom));
}

static inline double D_GTR1_isotropic(const vec3& h, const vec3& n, double a) {
    a = std::max(a, 1e-4);
    double ah2 = a * a;
    double cos2 = sqr(dot(h, n));
    double denom = pi * std::log(ah2) * (1.0 + (ah2 - 1.0) * cos2);

    return (ah2 - 1.0) / denom;
}

static inline double smithG1_GGX_aniso(const vec3& n, const vec3& v, const vec3& t, const vec3& b, double ax, double ay) {
    double vx = dot(v, t);
    double vy = dot(v, b);
    double vz = dot(v, n);

    double lambda = 0.0;
    double tan2 = (1.0 - sqr(vz)) / std::max(sqr(vz), 1e-7);

    if (tan2 <= 0.0) return 1.0;

    double cos2phi = (vx * vx) / std::max(vx * vx + vy * vy, 1e-8);
    double sin2phi = 1.0 - cos2phi;
    double a2 = cos2phi * sqr(ax) + sin2phi * sqr(ay);
    double root = 1.0 + a2 * tan2;

    lambda = (-1.0 + std::sqrt(root)) * 0.5;
    return 1.0 / (1.0 + lambda);
}

static inline double smithG_GGX_aniso(const vec3& n, const vec3& v, const vec3& l, const vec3& t, const vec3& b, double ax, double ay) {
    return smithG1_GGX_aniso(n, v, t, b, ax, ay) * smithG1_GGX_aniso(n, l, t, b, ax, ay);
}

static inline color diffuse_burley(const color& baseColor, double roughness, double cosThetaL, double cosThetaV, double cosThetaD) {
    double FD90 = 0.5 + 2.0 * roughness * sqr(cosThetaD);
    auto fresL = 1.0 + (FD90 - 1.0) * std::pow(1.0 - saturate(cosThetaL), 5.0);
    auto fresV = 1.0 + (FD90 - 1.0) * std::pow(1.0 - saturate(cosThetaV), 5.0);
    return (baseColor / pi) * (fresL * fresV);
}

static inline color sheen_lobe(const color& baseColor, double sheen, double sheenTint, double cosThetaD) {
    if (sheen <= 0.0) return color(0, 0, 0);
    double lum = std::max(luminance(baseColor), 1e-4);
    color tint = color(baseColor.x() / lum, baseColor.y() / lum, baseColor.z() / lum);
    color Cs = (1.0 - sheenTint) * color(1, 1, 1) + sheenTint * tint;
    double f = std::pow(1.0 - saturate(cosThetaD), 5.0);
    return sheen * Cs * f;
}

static inline void disney_aniso_params(double roughness, double anisotropic, double& ax, double& ay) {
    roughness = std::max(roughness, 1e-4);
    double aspect = std::sqrt(1.0 - 0.9 * anisotropic);
    ax = std::max(1e-4, (roughness * roughness) / aspect);
    ay = std::max(1e-4, (roughness * roughness) * aspect);
}

static inline color disney_F0(const color& baseColor, double metallic, double specular, double specularTint) {
    double lum = std::max(luminance(baseColor), 1e-4);
    color tint = color(baseColor.x() / lum, baseColor.y() / lum, baseColor.z() / lum);
    color F0_dielectric = (0.08 * specular) * ((1.0 - specularTint) * color(1, 1, 1) + specularTint * tint);

    return (1.0 - metallic) * F0_dielectric + metallic * baseColor;
}

static inline void disney_clearcoat_params(double clearcoat, double clearcoatGloss, double& weight, double& a_gtr1, double& F0_cc) {
    weight = 0.25 * clearcoat;
    a_gtr1 = std::max(1e-3, 0.1 * std::pow(0.1, clearcoatGloss));
    F0_cc = 0.04;
}

static inline vec3 sample_GTR2_aniso_halfvector(const vec3& n, const vec3& t, const vec3& b, double ax, double ay) {
    double u1 = random_double();
    double u2 = random_double();

    double phi = 2.0 * pi * u1;
    double cos_phi = ax * std::cos(phi);
    double sin_phi = ay * std::sin(phi);

    double r = std::sqrt(u2 / (1.0 - u2));

    vec3 hproj = r * (cos_phi * t + sin_phi * b);
    vec3 h0 = hproj + n;

    return unit_vector(h0);
}

static inline vec3 sample_GTR1_isotropic_halfvector(const vec3& n, const vec3& t, const vec3& b, double a) {
    double u1 = random_double();
    double u2 = random_double();
    double phi = 2.0 * pi * u1;
    double cos_theta = std::sqrt((1.0 - std::pow(a, 2.0 * u2)) / (1.0 - a * a));
    double sin_theta = std::sqrt(std::max(0.0, 1.0 - cos_theta * cos_theta));

    vec3 h = std::cos(phi) * sin_theta * t + std::sin(phi) * sin_theta * b + cos_theta * n;
    return unit_vector(h);
}

static inline vec3 refract(const vec3& uv, const vec3& n, double etai_over_etat) {
    double cos_theta = std::fmin(dot(-uv, n), 1.0);
    vec3 r_out_perp = etai_over_etat * (uv + cos_theta * n);
    vec3 r_out_parallel = -std::sqrt(std::fabs(1.0 - r_out_perp.length_squared())) * n;
    return r_out_perp + r_out_parallel;
}

// ------------------- Base pdf interface -------------------
class pdf {
public:
    virtual ~pdf() {}

    virtual double value(const vec3& direction) const = 0;
    virtual vec3 generate() const = 0;
};

// ------------------- general shape pdf -------------------
class shape_pdf : public pdf {

    const shape& objects;
    point3 origin;

public:
    shape_pdf(const shape& objects, const point3& origin);
    double value(const vec3& direction) const override;
    vec3 generate() const override;
};

// ------------------- disney material pdf -------------------

// Which lobe generate() sampled last. Mutable so tracer can read it
// after generate() and decide per-bounce whether to run NEE.
enum class DisneyLobe { Diffuse, Specular, Clearcoat };

class disney_mixture_pdf : public pdf {
public:
    disney_mixture_pdf(const hit_record& rec,
        const color& baseColor,
        double metallic, double specular, double specularTint,
        double roughness, double anisotropic,
        double sheen, double sheenTint,
        double clearcoat, double clearcoatGloss);

    double value(const vec3& direction) const override;
    vec3 generate() const override;
    void set_view(const vec3& wo);

    // Set by generate() so the tracer can query which lobe was sampled
    mutable DisneyLobe last_lobe = DisneyLobe::Diffuse;

private:
    // Geometry
    point3 p;
    vec3 n, t, b;
    mutable vec3 view;

    // Material params
    color baseColor;
    double metallic, specular, specularTint;
    double roughness, anisotropic;
    double sheen, sheenTint;
    double clearcoat, clearcoatGloss;

    // Weights
    double wd = 0, ws = 0, wc = 0;

    // Helpers
    static void build_frame(const vec3& n, vec3& t, vec3& b);
    static vec3 to_world(const vec3& local, const vec3& n, const vec3& t, const vec3& b);
    static vec3 random_cosine_direction();
};