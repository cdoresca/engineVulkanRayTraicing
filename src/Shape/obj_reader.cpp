#include "obj_reader.h"
#include "utility/color.h"

#include <fstream>
#include <sstream>
#include <iostream>
#include <map>

inline mat convert_tinyobj_material(const tinyobj::material_t& m, const std::string& baseDir) {
    mat material;
    material.name = m.name;
    material.Ns = m.shininess;
    material.Ni = m.ior;
    material.illum = (int)m.illum;
    material.d = m.dissolve;
    material.Tr = 0.0;

    material.Ka = color(m.ambient[0], m.ambient[1], m.ambient[2]);
    material.Kd = color(m.diffuse[0], m.diffuse[1], m.diffuse[2]);
    material.Ks = color(m.specular[0], m.specular[1], m.specular[2]);
    material.Ke = color(m.emission[0], m.emission[1], m.emission[2]);
    material.Tf = color(m.transmittance[0], m.transmittance[1], m.transmittance[2]);

    // Texture paths — prepend baseDir so they're absolute / relative to .obj
    auto resolve = [&](const std::string& p) -> std::string {
        return p.empty() ? "" : baseDir + p;
        };

    material.map_Kd = resolve(m.diffuse_texname);
    material.map_Ks = resolve(m.specular_texname);
    material.map_Ke = resolve(m.emissive_texname);
    material.map_Ns = resolve(m.specular_highlight_texname); // Ns map
    material.map_d = resolve(m.alpha_texname);
    material.map_bump = resolve(m.bump_texname.empty() ? m.normal_texname : m.bump_texname);

    return material;
}

bool loadOBJ(
    const std::string& path,
    vector<mesh>& outMeshes,
    vector<mat>& outMats)
{
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t>    shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;

    std::string baseDir = path.substr(0, path.find_last_of("/\\") + 1);

    bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials,
        &warn, &err, path.c_str(), baseDir.c_str());
    if (!warn.empty()) std::cout << warn << "\n";
    if (!err.empty())  std::cerr << err << "\n";
    if (!ret) return false;

    // ---- Materials ----
    //outMats.clear();
    for (const auto& m : materials)
        outMats.push_back(convert_tinyobj_material(m, baseDir));

    // ---- Meshes ----
    outMeshes.clear();
    for (const auto& shape : shapes) {
        std::map<std::tuple<int, int, int>, uint32_t> vertexToIndex;
        mesh s{ .name = shape.name };

        size_t index_offset = 0;
        for (size_t f = 0; f < shape.mesh.num_face_vertices.size(); f++) {
            size_t fv = shape.mesh.num_face_vertices[f];

            int mat_id = shape.mesh.material_ids[f];
            if (f == 0) {
                if (mat_id >= 0 && mat_id < (int)outMats.size())
                    s.mat_name = outMats[mat_id].name;
            }

            for (size_t v = 0; v < fv; v++) {
                tinyobj::index_t idx = shape.mesh.indices[index_offset + v];
                auto key = std::make_tuple(idx.vertex_index, idx.normal_index, idx.texcoord_index);

                if (vertexToIndex.count(key) == 0) {
                    vertex vx{};
                    vx.mat_id = mat_id != -1? mat_id:0;
                    vx.pos = glm::vec3(
                        attrib.vertices[3 * idx.vertex_index + 0],
                        attrib.vertices[3 * idx.vertex_index + 1],
                        attrib.vertices[3 * idx.vertex_index + 2]);

                    if (idx.normal_index >= 0)
                        vx.normal = glm::vec3(
                            attrib.normals[3 * idx.normal_index + 0],
                            attrib.normals[3 * idx.normal_index + 1],
                            attrib.normals[3 * idx.normal_index + 2]);
                        

                    // UV — write 0,0 if the .obj has no texcoords
                    if (idx.texcoord_index >= 0) {
                        vx.uv = glm::vec2(
                            attrib.texcoords[2 * idx.texcoord_index + 0],
                            attrib.texcoords[2 * idx.texcoord_index + 1]);
                    }
                    else {
                        vx.uv = glm::vec2(0.0f, 0.0f);
                    }

                    vertexToIndex[key] = (uint32_t)s.vertices.size();
                    s.vertices.push_back(vx);
                }

                s.indices.push_back(vertexToIndex[key]);
            }
            index_offset += fv;
        }

        s.transform = glm::mat4(1.0f);
        outMeshes.push_back(s);
    }

    return true;
}