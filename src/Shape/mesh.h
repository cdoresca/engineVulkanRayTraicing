#pragma once

#define GLM_ENABLE_EXPERIMENTAL
#include <vulkan/vulkan.h>
#include <glm/glm.hpp>
#include <unordered_map>

#include <glm/gtx/hash.hpp>

#include <array>
#include <vector>
#include <string>
#include "utility/color.h"


using namespace std;

/**
 * @brief 
 * le struct vertex contient la position, la normale et l'id du matériel
 */
struct vertex {
    glm::vec3 pos;
    float pad;
    glm::vec3 normal;
 
    uint32_t mat_id;

    glm::vec2 uv;
    glm::vec2 pad3;

    static VkVertexInputBindingDescription getBindingDescription();
    static array<VkVertexInputAttributeDescription, 2> getAttributeDescriptions();

};

struct mat {
    std::string name;

    // Standard MTL properties
    color Ka;           // Ambient color
    color Kd;           // Diffuse color (base)
    color Ks;           // Specular color
    color Ke;           // Emission color
    color Tf = color(1.0, 1.0, 1.0);
    double Ns;          // Specular exponent / shininess (0-1000)
    double d;           // Dissolve / opacity (0-1)
    double Tr;          // Transparency (0-1, Tr = 1 - d)
    double Ni;          // Index of refraction
    int illum;          // Illumination model (0-10)

    std::string map_Kd;    // diffuse / base color
    std::string map_Ks;    // specular
    std::string map_Ke;    // emission
    std::string map_Ns;    // roughness
    std::string map_d;     // alpha / opacity
    std::string map_bump;  // normal / bump map
};

/**
 * @brief 
 * Le struct MatGpu contient le diffuse et l'émissive d'un matériel envoyer sur le 
 * processeur graphique.
 */
struct MatGpu {
    glm::vec4 diffuse;
    glm::vec4 emissive;
};

/**
 * @brief 
 * Le struct node représente la graĥe hiéarchique de la scène.
 */
struct  node {
    std::string name;
    node* parent;
    vector<node*> child;
    glm::mat4 model;
};

glm::mat4 getMatrixParent(node* them);
/**
 * @brief 
 * Le struct mesh représente les informations d'un mesh (vertices, indices)
 */
struct mesh {
    std::string name;
    std::string mat_name;
    std::vector<vertex> vertices;
    std::vector<uint32_t> indices;
    glm::mat4 transform;
    node* him;
    bool hide;
};