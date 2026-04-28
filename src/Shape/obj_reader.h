#pragma once

#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include "utility/vec3.h"

#include "mesh.h"

#include <tiny_obj_loader.h>

inline mat convert_tinyobj_material(const tinyobj::material_t& tobj_mat);

bool loadOBJ(
    const std::string& path,
    std::vector<mesh>& outVertices,
    vector<mat>& outMats
);