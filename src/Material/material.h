#pragma once
#include "pdf.h"
#include "Texture/texture.h"
#include "utility/ray.h"

using std::shared_ptr;

class scatter_record {
public:
    color attenuation;
    shared_ptr<pdf> pdf_ptr;
    bool skip_pdf;
    ray skip_pdf_ray;
};

// ------------------- Base material interface -------------------
class material {
public:
    virtual ~material() = default;

    virtual bool is_emissive() const;
    virtual bool scatter(const ray& r_in, const hit_record& rec, scatter_record& srec) const;
    virtual double scattering_pdf(const ray& r_in, const hit_record& rec, const ray& scattered) const;
    virtual color emitted(const ray& r_in, const hit_record& rec, double u, double v, const point3& p) const;
};

// ------------------- disney_principled material -------------------
class disney_principled : public material {
public:
    // Parameters
    double subsurface;
    double metallic;
    double specular;
    double specularTint;
    double roughness;
    double anisotropic;
    double sheen;
    double sheenTint;
    double clearcoat;
    double clearcoatGloss;
    double emissionStrength;
    double specTrans;
    double ior;

    color emission;
    shared_ptr<texture> tex;

    // Constructors
    disney_principled(
        const color& base,
        double subsurface = 0.0,
        double metallic = 0.0,
        double specular = 0.5,
        double specularTint = 0.0,
        double roughness = 0.5,
        double anisotropic = 0.0,
        double sheen = 0.0,
        double sheenTint = 0.5,
        double clearcoat = 0.0,
        double clearcoatGloss = 0.0,
        double specTrans = 0.0,
        double ior = 1.5,
        const color& emission = color(0, 0, 0),
        double emissionStrength = 1.0
    );

    disney_principled(
        shared_ptr<texture> baseTex,
        double subsurface = 0.0,
        double metallic = 0.0,
        double specular = 0.5,
        double specularTint = 0.0,
        double roughness = 0.5,
        double anisotropic = 0.0,
        double sheen = 0.0,
        double sheenTint = 0.5,
        double clearcoat = 0.0,
        double clearcoatGloss = 0.0,
        double specTrans = 0.0,
        double ior = 1.5,
        const color& emission = color(0, 0, 0),
        double emissionStrength = 1.0
    );

    // Overrides
    bool   is_emissive()       const override;
    bool   is_highly_specular()const;
    bool   scatter(const ray& r_in, const hit_record& rec, scatter_record& srec) const override;
    double scattering_pdf(const ray& r_in, const hit_record& rec, const ray& scattered) const override;
    color  emitted(const ray& r_in, const hit_record& rec, double u, double v, const point3& p) const override;

    color  evaluate_brdf(const vec3& n, const vec3& t, const vec3& b,
        const color& baseColor,
        const vec3& wo, const vec3& wi) const;

private:
    static double reflectance(double cosine, double ri);
};