#pragma once

#include "Rendering/camera.h"
#include "Rendering/tracer.h"
#include "Shape/shape_list.h"
#include "Shape/mesh.h"
#include "Sample/sampler.h"
#include "UI/object.h"

#include <atomic>
#include <glm/glm.hpp>

struct settings {
	double aspectRatio = 16/9;
    int imgWidth = 1920;
    int samplesPerPixel = 1;
    int maxDepth = 15;
    int vfov = 40;
    int defocusAngle = 0;
    float position[3] = { 0, 0, 0 }; //point
    float lookat[3] = { 0, 0, 0 }; //point
    float vup[3] = { 0, 1, 0 }; //vect
    char name[48] =  "";
};

class world {
public:
    camera cam;
    shape_list scene;
    shape_list lights;
    color background;
    std::unique_ptr<tracer> tracer_type;
    std::unique_ptr<sampler> sampler_type;

    ~world();

    void build(settings set);
    void add_shapes(vector<mesh> outMesh,vector<mat> outMats);
    void add_camera(settings set);
    void render(std::atomic<int>& rows_done);
};


struct instanceInfo {
    glm::mat4 transform;
    uint32_t material;
    uint32_t meshIndex;

};