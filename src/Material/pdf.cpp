#include "pdf.h"

// -----------------------------------------------------------------------
// shape_pdf
// -----------------------------------------------------------------------
shape_pdf::shape_pdf(const shape& objects, const point3& origin)
    : objects(objects), origin(origin) {
}

double shape_pdf::value(const vec3& direction) const {
    return objects.pdf_value(origin, direction);
}

vec3 shape_pdf::generate() const {
    return objects.random(origin);
}

// -----------------------------------------------------------------------
// disney_mixture_pdf
// -----------------------------------------------------------------------

// Build a local orthonormal frame given a normal.
void disney_mixture_pdf::build_frame(const vec3& n, vec3& t, vec3& b) {
    double sign = std::copysign(1.0, n.z());
    double a = -1.0 / (sign + n.z());
    double bv = n.x() * n.y() * a;
    t = vec3(1.0 + sign * n.x() * n.x() * a, sign * bv, -sign * n.x());
    b = vec3(bv, sign + n.y() * n.y() * a, -n.y());
}

vec3 disney_mixture_pdf::to_world(const vec3& local, const vec3& n, const vec3& t, const vec3& b) {
    return local.x() * t + local.y() * b + local.z() * n;
}

vec3 disney_mixture_pdf::random_cosine_direction() {
    double r1 = random_double();
    double r2 = random_double();
    double phi = 2.0 * pi * r1;
    double sqrtr2 = std::sqrt(r2);
    return vec3(std::cos(phi) * sqrtr2, std::sin(phi) * sqrtr2, std::sqrt(1.0 - r2));
}

// ---------- Constructor ----------
disney_mixture_pdf::disney_mixture_pdf(
    const hit_record& rec,
    const color& baseColor,
    double metallic_, double specular_, double specularTint_,
    double roughness_, double anisotropic_,
    double sheen_, double sheenTint_,
    double clearcoat_, double clearcoatGloss_)
    : p(rec.p), n(rec.normal),
    baseColor(baseColor),
    metallic(metallic_), specular(specular_), specularTint(specularTint_),
    roughness(roughness_), anisotropic(anisotropic_),
    sheen(sheen_), sheenTint(sheenTint_),
    clearcoat(clearcoat_), clearcoatGloss(clearcoatGloss_)
{
    build_frame(n, t, b);

    // --- Lobe weights (used to pick which lobe to sample) ---

    double lum = std::max(luminance(baseColor), 1e-4);

    wd = (1.0 - metallic) * lum;
    ws = 1.0;          
    wc = 0.25 * clearcoat;

    double total = wd + ws + wc;
    wd /= total;
    ws /= total;
    wc /= total;
}

void disney_mixture_pdf::set_view(const vec3& wo) {
    view = unit_vector(wo);
}

// ---------- generate ----------
vec3 disney_mixture_pdf::generate() const {
    double rng = random_double();

    if (rng < wd) {
        // --- Diffuse: cosine-weighted hemisphere sample ---
        last_lobe = DisneyLobe::Diffuse;
        vec3 local = random_cosine_direction();
        return to_world(local, n, t, b);

    }
    else if (rng < wd + ws) {
        // --- Specular: sample GTR2 anisotropic half-vector, reflect view ---
        last_lobe = DisneyLobe::Specular;
        double ax, ay;
        disney_aniso_params(roughness, anisotropic, ax, ay);

        vec3 h = sample_GTR2_aniso_halfvector(n, t, b, ax, ay);
        // Reflect view around h
        vec3 wi = 2.0 * dot(view, h) * h - view;
        return unit_vector(wi);

    }
    else {
        // --- Clearcoat: sample GTR1 isotropic half-vector, reflect view ---
        last_lobe = DisneyLobe::Clearcoat;
        double ccWeight, a_gtr1, F0_cc;
        disney_clearcoat_params(clearcoat, clearcoatGloss, ccWeight, a_gtr1, F0_cc);

        vec3 h = sample_GTR1_isotropic_halfvector(n, t, b, a_gtr1);
        vec3 wi = 2.0 * dot(view, h) * h - view;
        return unit_vector(wi);
    }
}

// ---------- value  (mixture PDF evaluated at a given direction) ----------
double disney_mixture_pdf::value(const vec3& direction) const {
    vec3 wi = unit_vector(direction);

    double cosThetaL = cos_theta(n, wi);
    if (cosThetaL <= 0.0) return 0.0;

    vec3 h = unit_vector(view + wi);
    double cosThetaH = std::max(dot(h, n), 0.0);
    double cosThetaD = std::max(dot(wi, h), 0.0);

    // ---- Diffuse PDF: cosine-weighted hemisphere ----
    double pdf_d = cosThetaL / pi;

    // ---- Specular PDF: GTR2 aniso D, Jacobian |dh/dwi| = 1/(4(l·h)) ----
    double ax, ay;
    disney_aniso_params(roughness, anisotropic, ax, ay);
    double Ds = D_GTR2_aniso(h, n, t, b, ax, ay);
    // pdf_h = D * cos(theta_h), converted to wi: divide by 4*(l·h)
    double pdf_s = (Ds * cosThetaH) / std::max(4.0 * cosThetaD, 1e-7);

    // ---- Clearcoat PDF: GTR1 iso D ----
    double ccWeight, a_gtr1, F0_cc;
    disney_clearcoat_params(clearcoat, clearcoatGloss, ccWeight, a_gtr1, F0_cc);
    double Dc = D_GTR1_isotropic(h, n, a_gtr1);
    double pdf_c = (Dc * cosThetaH) / std::max(4.0 * cosThetaD, 1e-7);

    return wd * pdf_d + ws * pdf_s + wc * pdf_c;
}