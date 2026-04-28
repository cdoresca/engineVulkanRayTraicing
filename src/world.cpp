#include "world.h"

#include "shape/obj_shape.h"
#include "Material/material.h"
#include "Shape/obj_reader.h"

world::~world() = default;

void world::build(settings set) {
	background = SKY_BLUE;
	tracer_type = std::make_unique<path_tracing>();
	sampler_type = std::make_unique<stratified>(set.samplesPerPixel);

	add_camera(set);

	std::clog << "\33[2K\r" << std::flush;
}

//Notes : true absorption + thickness a ajouter dans le future

std::unordered_map<std::string, shared_ptr<material>>
parse_mtl_to_disney(const std::vector<mat>& outMats)
{
    std::unordered_map<std::string, shared_ptr<material>> materials;

    // Texture cache — keyed by resolved file path
    std::unordered_map<std::string, shared_ptr<texture>> tex_cache;
    auto get_image_tex = [&](const std::string& path) -> shared_ptr<texture> {
        if (path.empty()) return nullptr;
        auto it = tex_cache.find(path);
        if (it != tex_cache.end()) return it->second;
        auto tex = std::make_shared<image_texture>(path.c_str());
        tex_cache[path] = tex;
        return tex;
        };

    for (const auto& m : outMats) {

        // =====================================================
        // STEP 1: Build base texture (image or solid color)
        // =====================================================
        shared_ptr<texture> baseTex;
        if (!m.map_Kd.empty()) {
            baseTex = get_image_tex(m.map_Kd);
        }
        else {
            color baseColor = m.Kd;
            if (baseColor.x() == 0 && baseColor.y() == 0 && baseColor.z() == 0)
                baseColor = m.Ka;
            if (baseColor.x() == 0 && baseColor.y() == 0 && baseColor.z() == 0)
                baseColor = color(0.5, 0.5, 0.5);
            baseTex = std::make_shared<solid_color>(baseColor);
        }

        // Representative color for scalar heuristics below
        // solid_color ignores uv, image_texture samples at (0,0) as an approximation
        color baseColor = baseTex->value(0, 0, point3(0, 0, 0));

        // =====================================================
        // STEP 2: Roughness from Ns (specular exponent)
        // =====================================================
        double roughness = std::sqrt(2.0 / (m.Ns + 2.0));
        roughness = std::clamp(roughness, 0.0, 1.0);

        // =====================================================
        // STEP 3: Metallic from Ks
        // =====================================================
        double ks_intensity = (m.Ks.x() + m.Ks.y() + m.Ks.z()) / 3.0;

        double ks_deviation = std::abs(m.Ks.x() - m.Ks.y())
            + std::abs(m.Ks.y() - m.Ks.z());
        bool is_grayscale = ks_deviation < 0.1;

        double metallic = 0.0;
        if (ks_intensity > 0.6 && baseColor.length() < 0.2)
            metallic = 1.0;

        double specular = std::clamp(ks_intensity, 0.0, 1.0);
        double specularTint = is_grayscale ? 0.0 : 0.5;

        // =====================================================
        // STEP 4: Transmission
        // =====================================================
        double transparency = (m.Tr > 0.0) ? m.Tr : (1.0 - m.d);
        transparency = std::clamp(transparency, 0.0, 1.0);
        double specTrans = (transparency > 0.1) ? transparency : 0.0;

        // If transmissive, prefer Tf as base color when it's brighter than Kd
        if (specTrans > 0.1 && !m.map_Kd.empty()) {
            // Keep image texture — don't override with Tf
        }
        else if (specTrans > 0.1) {
            color tf = m.Tf;
            double tf_lum = (tf.x() + tf.y() + tf.z()) / 3.0;
            double kd_lum = (baseColor.x() + baseColor.y() + baseColor.z()) / 3.0;
            if (tf_lum > kd_lum + 0.1) {
                baseColor = tf;
                baseTex = std::make_shared<solid_color>(tf);
            }
        }

        // =====================================================
        // STEP 5: Index of refraction
        // =====================================================
        double ior = std::clamp((m.Ni > 0.0) ? m.Ni : 1.5, 1.0, 3.0);

        // =====================================================
        // STEP 6: Disney parameters (defaults)
        // =====================================================
        double subsurface = 0.0;
        double anisotropic = 0.0;
        double sheen = 0.0;
        double sheenTint = 0.0;
        double clearcoat = 0.0;
        double clearcoatGloss = 1.0;

        // =====================================================
        // STEP 7: Illumination model overrides
        // =====================================================
        switch (m.illum) {
        case 0: // Ambient only — flat, no specularity
            metallic = 0.0;
            specular = 0.0;
            roughness = 1.0;
            break;

        case 1: // Diffuse only
            metallic = 0.0;
            specular = 0.0;
            roughness = std::max(0.5, roughness);
            break;

        case 2: // Diffuse + specular
            if (m.Ns > 300.0 && ks_intensity < 0.5 && baseColor.length() > 0.5) {
                clearcoat = 0.8;
                clearcoatGloss = 1.0 - roughness;
            }
            break;

        case 3: // Mirror / perfect reflection
            if (m.map_Kd.empty())
                baseTex = std::make_shared<solid_color>(m.Ks);
            baseColor = baseTex->value(0, 0, point3(0, 0, 0));
            metallic = 1.0;
            roughness = std::min(0.001, roughness);
            specular = 1.0;
            break;

        case 4: // Diffuse + specular + ray-traced reflection
            if (ks_intensity > 0.1) {
                clearcoat = ks_intensity * 0.5;
                clearcoatGloss = 1.0 - roughness;
            }
            break;

        case 5: // Refraction
            specTrans = std::max(0.3, specTrans);
            break;

        case 6: // Refraction on flat surface
            specTrans = std::max(0.5, specTrans);
            roughness = std::min(0.2, roughness);
            break;

        case 7: // Reflection + refraction
            specTrans = std::max(specTrans, 0.85);
            metallic = 0.0;
            roughness = std::min(roughness, 0.05);
            specular = std::max(specular, 0.5);
            break;

        case 8: // Diffuse + edge highlight
            anisotropic = 0.1;
            break;

        case 9: // Diffuse + partial specularity
            sheen = 0.3;
            break;

        case 10: // Fresnel reflection
            specularTint = 0.8;
            break;

        default:
            break;
        }

        // =====================================================
        // STEP 8: Emission
        // =====================================================
        shared_ptr<texture> emitTex = get_image_tex(m.map_Ke);
        if (!m.map_Ke.empty())
            emitTex = std::make_shared<image_texture>(m.map_Ke.c_str());

        color emission = m.Ke;
        double emissionStrength = (emission.x() + emission.y() + emission.z()) / 3.0;
        if (emissionStrength > 0.0) {
            emission = emission * (1.0 / emissionStrength);
            emissionStrength = std::min(10.0, emissionStrength);
        }
        else {
            emissionStrength = 0.0;
        }

        // =====================================================
        // STEP 9: Clamp all parameters
        // =====================================================
        metallic = std::clamp(metallic, 0.0, 1.0);
        specular = std::clamp(specular, 0.0, 1.0);
        specularTint = std::clamp(specularTint, 0.0, 1.0);
        roughness = std::clamp(roughness, 0.0, 1.0);
        anisotropic = std::clamp(anisotropic, 0.0, 1.0);
        sheen = std::clamp(sheen, 0.0, 1.0);
        sheenTint = std::clamp(sheenTint, 0.0, 1.0);
        clearcoat = std::clamp(clearcoat, 0.0, 1.0);
        clearcoatGloss = std::clamp(clearcoatGloss, 0.0, 1.0);
        specTrans = std::clamp(specTrans, 0.0, 1.0);

        if (metallic > 0.5)
            subsurface = 0.0; // No subsurface for metals

        // =====================================================
        // STEP 10: Construct Disney material
        // =====================================================
		auto material = std::make_shared<disney_principled>(
			baseTex,
			subsurface,      // Typically 0.0 for most MTL materials
			metallic,        // Derived from Ks and illum
			specular,        // Derived from Ks intensity
			specularTint,    // Derived from Ks color
			roughness,       // Derived from Ns (inverted)
			anisotropic,     // From illum model
			sheen,           // From illum model
			sheenTint,       // From illum model (typically 0.5)
			clearcoat,       // From illum model
			clearcoatGloss,  // Related to roughness
			specTrans,       // Derived from d/Tr
			ior,             // From Ni
			emission,        // From Ke
			emissionStrength // Computed from Ke magnitude
		);

		materials[m.name] = material;
	}

    return materials;
}

void world::add_shapes(vector<mesh> outMesh,vector<mat> outMats) {

	scene.add(make_shared<obj_shape>(
    point3(0, 0, 0),
    outMesh, 
    parse_mtl_to_disney(outMats),
    make_shared<disney_principled>(WHITE), 
    lights));

	outMesh.clear();
	outMats.clear();

	outMesh.shrink_to_fit();
	outMats.shrink_to_fit();
}

void world::add_camera(settings set) {

    cam.aspect_ratio = set.aspectRatio;
    cam.image_width = set.imgWidth;
    cam.max_depth = set.maxDepth;

    cam.lookfrom = vec3(set.position[0], set.position[1], set.position[2]);
    cam.lookat = vec3(set.lookat[0], set.lookat[1], set.lookat[2]);
    cam.vup = vec3(set.vup[0], set.vup[1], set.vup[2]);

    cam.vfov = set.vfov;
    cam.defocus_angle = set.defocusAngle;
    
    strcpy(cam.filename, set.name);
}


void world::render(std::atomic<int>& rows_done) {
	cam.render(*this, &rows_done);
}