#include "material.h"
#include "pdf.h"

// -----------------------------------------------------------------------
// material base implementations (default no-ops)
// -----------------------------------------------------------------------
bool material::is_emissive() const { return false; }
bool material::scatter(const ray&, const hit_record&, scatter_record&) const { return false; }
double material::scattering_pdf(const ray&, const hit_record&, const ray&) const { return 0.0; }
color material::emitted(const ray&, const hit_record&, double, double, const point3&) const {
    return color(0, 0, 0);
}

// -----------------------------------------------------------------------
// Constructors
// -----------------------------------------------------------------------
disney_principled::disney_principled(
    const color& base,
    double subsurface, double metallic, double specular, double specularTint,
    double roughness, double anisotropic, double sheen, double sheenTint,
    double clearcoat, double clearcoatGloss,
    double specTrans, double ior,
    const color& emission, double emissionStrength)
    : subsurface(subsurface), metallic(metallic), specular(specular),
    specularTint(specularTint), roughness(roughness), anisotropic(anisotropic),
    sheen(sheen), sheenTint(sheenTint), clearcoat(clearcoat),
    clearcoatGloss(clearcoatGloss), emissionStrength(emissionStrength),
    specTrans(specTrans), ior(ior), emission(emission),
    tex(std::make_shared<solid_color>(base))
{
}

disney_principled::disney_principled(
    shared_ptr<texture> baseTex,
    double subsurface, double metallic, double specular, double specularTint,
    double roughness, double anisotropic, double sheen, double sheenTint,
    double clearcoat, double clearcoatGloss,
    double specTrans, double ior,
    const color& emission, double emissionStrength)
    : subsurface(subsurface), metallic(metallic), specular(specular),
    specularTint(specularTint), roughness(roughness), anisotropic(anisotropic),
    sheen(sheen), sheenTint(sheenTint), clearcoat(clearcoat),
    clearcoatGloss(clearcoatGloss), emissionStrength(emissionStrength),
    specTrans(specTrans), ior(ior), emission(emission), tex(baseTex)
{
}

// -----------------------------------------------------------------------
// Helpers
// -----------------------------------------------------------------------
bool disney_principled::is_emissive() const {
    return emissionStrength > 0.0 && (emission.x() > 0 || emission.y() > 0 || emission.z() > 0);
}

bool disney_principled::is_highly_specular() const {
    return metallic > 0.9 || specTrans > 0.5;
}

double disney_principled::reflectance(double cosine, double ri) {
    // Schlick approximation for dielectric reflectance
    double r0 = (1 - ri) / (1 + ri);
    r0 = r0 * r0;
    return r0 + (1 - r0) * std::pow(1 - cosine, 5.0);
}

color disney_principled::emitted(const ray& r_in, const hit_record& rec, double u, double v, const point3& p) const {
    if (!rec.front_face) return color(0, 0, 0);
    return emissionStrength * emission;
}


// -----------------------------------------------------------------------
// evaluate_brdf  –  full Disney "principled" reflectance
//
//   n, t, b  : shading normal / tangent / bitangent (world space, unit)
//   wo       : outgoing direction (toward camera, world space)
//   wi       : incoming direction (toward light, world space)
//
// Returns the BRDF value f(wo, wi).  The caller is responsible for the
// cos(theta_i) * Li weighting that is part of the rendering equation.
// -----------------------------------------------------------------------
color disney_principled::evaluate_brdf(
    const vec3& n, const vec3& t, const vec3& b,
    const color& baseColor,
    const vec3& wo, const vec3& wi) const
{
    // ---- Dot products we'll need everywhere ----------------------------
    double cosThetaL = cos_theta(n, wi);   // n·l
    double cosThetaV = cos_theta(n, wo);   // n·v

    // Both directions must be on the same side as the normal for reflection.
    if (cosThetaL <= 0.0 || cosThetaV <= 0.0) return color(0, 0, 0);

    // Half vector
    vec3 h = unit_vector(wo + wi);

    double cosThetaH = std::max(dot(h, n), 0.0);  // n·h
    double cosThetaD = std::max(dot(wi, h), 0.0);  // l·h  (= v·h by symmetry)

    // ---- Anisotropic roughness parameters  (Section 5.4 + Addenda) ----
    double ax, ay;
    disney_aniso_params(roughness, anisotropic, ax, ay);

    // ---- Specular D  (GTR2 anisotropic, eq. 13) -----------------------
    double Ds = D_GTR2_aniso(h, n, t, b, ax, ay);

    // ---- Specular G  (Smith GGX anisotropic, Walter 2007 + remapping) -
    //   Per the paper (section 5.6) we remap roughness for G:
    //      alpha_g = (0.5 + roughness/2)^2
    double roughnessG = 0.5 + roughness * 0.5;
    double axG = roughnessG * roughnessG;   // isotropic for G (paper uses GGX G)
    // Use the anisotropic form with axG, ayG derived from remapped roughness
    double ayG = axG;
    double Gs = smithG_GGX_aniso(n, wo, wi, t, b, axG, ayG);

    // ---- Specular F  (Schlick, section 5.5) ---------------------------
    color F0 = disney_F0(baseColor, metallic, specular, specularTint);
    color Fs = fresnel_schlick(F0, cosThetaD);

    // ---- Primary specular lobe ----------------------------------------
    color spec = (Ds * Gs * Fs) / std::max(4.0 * cosThetaL * cosThetaV, 1e-7);

    // ---- Diffuse  (Burley, section 5.3) ------------------------------
    //   Blended with subsurface (Hanrahan-Krueger inspired)
    color fd = diffuse_burley(baseColor, roughness, cosThetaL, cosThetaV, cosThetaD);

    // Subsurface approximation (HK-inspired blend)
    // At subsurface=1 we use a flatter, slightly different shape.
    color fss;
    {
        double Fss90 = roughness * sqr(cosThetaD);
        double FssL = 1.0 + (Fss90 - 1.0) * std::pow(1.0 - saturate(cosThetaL), 5.0);
        double FssV = 1.0 + (Fss90 - 1.0) * std::pow(1.0 - saturate(cosThetaV), 5.0);
        double ss = 1.25 * (FssL * FssV * (1.0 / (cosThetaL + cosThetaV) - 0.5) + 0.5);
        fss = (baseColor / pi) * ss;
    }
    color diffuse = (1.0 - subsurface) * fd + subsurface * fss;

    // Metallic materials have no diffuse
    diffuse = (1.0 - metallic) * diffuse;

    // ---- Sheen lobe  (Addenda / section 5.2) -------------------------
    color sheenVal = sheen_lobe(baseColor, sheen, sheenTint, cosThetaD);
    sheenVal = (1.0 - metallic) * sheenVal;

    // ---- Clearcoat lobe  (GTR1, section 5.4) -------------------------
    color cc(0, 0, 0);
    if (clearcoat > 0.0) {
        double ccWeight, a_gtr1, F0_cc;
        disney_clearcoat_params(clearcoat, clearcoatGloss, ccWeight, a_gtr1, F0_cc);

        double Dc = D_GTR1_isotropic(h, n, a_gtr1);

        // Clearcoat G: fixed roughness = 0.25 (section 5.6)
        double axC = 0.25 * 0.25;
        double Gc = smithG_GGX_aniso(n, wo, wi, t, b, axC, axC);

        // Fresnel for clearcoat (F0 = 0.04, achromatic)
        color F0c(F0_cc, F0_cc, F0_cc);
        color Fc = fresnel_schlick(F0c, cosThetaD);

        cc = ccWeight * (Dc * Gc * Fc) / std::max(4.0 * cosThetaL * cosThetaV, 1e-7);
    }

    // ---- Specular transmission  (thin-surface dielectric) ------------
    //   Simple hack: when specTrans > 0, add a transmitted lobe using
    //   a refracted half-vector. For a path tracer the proper way is to
    //   sample transmission separately; here we add its BRDF contribution.
    //   (Full BSDF transmission is handled in scatter() below.)

    // ---- Combine lobes -----------------------------------------------
    return diffuse + sheenVal + spec + cc;
}

// -----------------------------------------------------------------------
// scatter  –  importance-sample a direction and fill scatter_record
// -----------------------------------------------------------------------
bool disney_principled::scatter(
    const ray& r_in, const hit_record& rec, scatter_record& srec) const
{
    // Evaluate baseColor at hit point
    color baseColor = tex->value(rec.u, rec.v, rec.p);

    // ---- Handle specular transmission ---------------------------------
    //   If specTrans > 0 we probabilistically choose to transmit.
    if (specTrans > 0.0) {
        double cosThetaI = dot(-r_in.direction(), rec.normal);
        bool front_face = cosThetaI > 0.0;
        double ri = front_face ? (1.0 / ior) : ior;
        vec3 unit_dir = unit_vector(r_in.direction());
        vec3 norm = front_face ? rec.normal : -rec.normal;

        double cos_theta_i = std::fmin(dot(-unit_dir, norm), 1.0);
        double refl_prob = reflectance(cos_theta_i, ri);

        // Mix transmission probability with specTrans
        double trans_prob = specTrans * (1.0 - refl_prob);

        if (random_double() < trans_prob) {
            // Transmit
            vec3 refracted_dir = refract(unit_dir, norm, ri);
            srec.skip_pdf = true;
            srec.skip_pdf_ray = ray(rec.p, refracted_dir);
            srec.attenuation = baseColor;  // tint by base color
            return true;
        }
    }

    // ---- Reflection path: use Disney mixture PDF ---------------------
    srec.skip_pdf = false;

    auto pdf = std::make_shared<disney_mixture_pdf>(
        rec, baseColor,
        metallic, specular, specularTint,
        roughness, anisotropic,
        sheen, sheenTint,
        clearcoat, clearcoatGloss);

    pdf->set_view(-r_in.direction());
    srec.pdf_ptr = pdf;

    // Attenuation is handled by evaluate_brdf / scattering_pdf in the integrator.
    // We set it to white here; the integrator multiplies by evaluate_brdf / pdf.
    srec.attenuation = color(1, 1, 1);
    return true;
}

// -----------------------------------------------------------------------
// scattering_pdf  –  returns p(wi | wo) for the mixture PDF
// -----------------------------------------------------------------------
double disney_principled::scattering_pdf(
    const ray& r_in, const hit_record& rec, const ray& scattered) const
{
    vec3 n = rec.normal;
    vec3 wo = -r_in.direction();
    vec3 wi = unit_vector(scattered.direction());

    double cosThetaL = cos_theta(n, wi);
    if (cosThetaL <= 0.0) return 0.0;

    color baseColor = tex->value(rec.u, rec.v, rec.p);

    // Build the same mixture PDF and evaluate it
    disney_mixture_pdf mpdf(
        rec, baseColor,
        metallic, specular, specularTint,
        roughness, anisotropic,
        sheen, sheenTint,
        clearcoat, clearcoatGloss);
    mpdf.set_view(wo);

    return mpdf.value(wi);
}