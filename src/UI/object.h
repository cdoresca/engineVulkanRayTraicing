#pragma once

#include <string>
#include <vector>
#include <memory>
#include <array>
#include <optional>
#include <Rendering/camera.h>
#include <vulkanWrapper/accelerationStructure.h>
using namespace std;

using Position = array<double, 3>;
using Scale = array<double, 3>;
using Rotation = array<double, 3>;
using Color = array<double, 3>;

struct ObjectInsertSet {
    Position position;
    Scale scale;
    Rotation rotation;
    optional<Color> color;
};

struct Material {
    float subsurface = 0.f;
    float metallic = 0.f;
    float specular = 0.f;
	float specularTint = 0.f;
    float roughness = 0.f;
	float anisotropic = 0.f;
    float sheen = 0.f;
	float sheenTint = 0.f;
    float clearcoat = 0.f;
    float clearcoatGloss = 0.f;
    float specTrans = 0.f;
    float ior = 0.f;
    array<float, 3> emission = { 0.f, 0.f, 0.f };
    float emissionStrength = 0.f;

};

//Object
class ui_object {
public:
    std::string name;
    Material material;
    std::string type;
    std::vector<ui_object*> children;
    bool open = false;
	bool visible = true;
    float objectMatrix[16] = 
    { 1.f, 0.f, 0.f, 0.f,
    0.f, 1.f, 0.f, 0.f,
    0.f, 0.f, 1.f, 0.f,
    0.f, 0.f, 0.f, 1.f };
    float position[3] = { 0.0f, 0.0f, 0.0f };
    float scale[3] = { 1.0f, 1.0f, 1.0f };
    float rotation[3] = { 0.0f, 0.0f, 0.0f };
    float color[3] = { 0.0f, 0.0f, 0.0f };
    node* him;
    acceleration* accel;
    ui_object();
    ui_object(std::string name);
    ui_object(std::string name,node* n, acceleration* a);

    virtual void show_details();
    virtual void show();
    virtual std::string save() { return ""; }
    virtual shared_ptr<ui_object> clone() { return std::make_shared<ui_object>(*this); }

    //For camera exclusively
    virtual float get_fov() { return 0.0f; };
    virtual bool get_viewDirty() { return false; };
    virtual float* get_lookat() { float nothing[] = { 0.0f, 0.f, 0.f }; return nothing; };
    virtual float* get_vup() { float nothing[] = { 0.0f, 0.f, 0.f }; return nothing; };
    virtual float* get_position() { float nothing[] = { 0.0f, 0.f, 0.f }; return nothing; };
    virtual float get_camDistance() { return 0.f; };
};

//Camera
//Always first object in the objects vector
class ui_camera : public ui_object {
public:
    float position[3] = { 50.f, 25.f, 50.f };
    float lookat[3] = { 0.f, 0.f, 0.f};
    float vup[3] = { 0.f, 1.f, 0.f };
    Camera* cam;
    //Imguizmo
    float fov = 27.f;
	bool viewDirty = false;

    ui_camera(std::string name);

    //Imguizmo
    float get_camDistance() override;
    float get_fov() override { return fov; };
    bool get_viewDirty() override { return viewDirty; };
    float* get_lookat() override { return lookat; };
    float* get_vup() override { return vup; };
    float* get_position() override { return position; };
    void show() override{}
    void show_details() override;
    std::string save() override;
    //std::shared_ptr<ui_object> clone() const override;
};

//Cube
class ui_cube : public ui_object {
public:
    ui_cube(std::string name);
    ui_cube(ObjectInsertSet insertSet, string name);

    std::string save() override;
    //std::shared_ptr<ui_object> clone() const override;
};

//Sphere
class ui_sphere : public ui_object {
public:
    ui_sphere(std::string name);
    ui_sphere(ObjectInsertSet insertSet, string name);

    std::string save() override;
    //std::shared_ptr<ui_object> clone() const override;
};

//Plane
class ui_plane : public ui_object {
public:
    ui_plane(std::string name);
    ui_plane(ObjectInsertSet insertSet, string name);

    std::string save() override;
    //std::shared_ptr<ui_object> clone() const override;
};

//Obj File
class ui_obj_file : public ui_object {
public:
    std::string path = "";

    ui_obj_file(std::string name);

    std::string save() override;
    //std::shared_ptr<ui_object> clone() const override;
};