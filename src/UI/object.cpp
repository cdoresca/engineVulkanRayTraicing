#include <string>
#include <imgui.h>
#include "object.h"
#include <vector>
#include <ImGuizmo.h>
#include <glm/gtc/type_ptr.hpp>
#include "UI.h"

//Object (Constructors)
ui_object::ui_object() {
    this->name = "default_name";
}

ui_object::ui_object(string name) {
    this->name = name;
}

ui_object::ui_object(std::string name, node* n, acceleration* a):name(name),him(n),accel(a)
{
}

void ui_object::show_details() {
    if (this->open) {
        ImGuizmo::DecomposeMatrixToComponents(glm::value_ptr(him->model), position, rotation, scale);

        ImGui::Text("Object details:");
        if(ImGui::InputFloat3("Position", (float*)&position) ||
            ImGui::InputFloat3("Rotation", (float*)&rotation) ||
            ImGui::InputFloat3("Scale", (float*)&scale)
         )
        {
            ImGuizmo::RecomposeMatrixFromComponents(position, rotation, scale, glm::value_ptr(him->model));
            accel->updateMesh(him);
            accel->rebuildTlas(VK_BUILD_ACCELERATION_STRUCTURE_MODE_UPDATE_KHR);
        }

        ImGui::SliderFloat("Subsurface", &material.subsurface, 0.f, 1.f);
        ImGui::SliderFloat("Metallic", &material.metallic, 0.f, 1.f);
        ImGui::SliderFloat("Specular", &material.specular, 0.f, 1.f);
        ImGui::SliderFloat("Specular Tint", &material.specularTint, 0.f, 1.f);
        ImGui::SliderFloat("Roughness", &material.roughness, 0.f, 1.f);
        ImGui::SliderFloat("Anisotropic", &material.anisotropic, 0.f, 1.f);
        ImGui::SliderFloat("Sheen", &material.sheen, 0.f, 1.f);
        ImGui::SliderFloat("Sheen Tint", &material.sheenTint, 0.f, 1.f);
        ImGui::SliderFloat("Clearcoat", &material.clearcoat, 0.f, 1.f);
        ImGui::SliderFloat("Clearcoat Gloss", &material.clearcoatGloss, 0.f, 1.f);
        ImGui::SliderFloat("Spec Trans", &material.specTrans, 0.f, 1.f);
        ImGui::SliderFloat("Ior", &material.ior, 0.f, 1.f);
        ImGui::ColorEdit3("Emission", (float*)&material.emission);
        ImGui::SliderFloat("Emission Strength", &material.emissionStrength, 0.f, 1.f);
    }
 
}

void ui_object::show()
{
    if (visible) {
        accel->addInstance(him);
        accel->rebuildTlas(VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR);
    }
    else {
        accel->remove(him);
        accel->rebuildTlas(VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR);
    }
}

//Camera
ui_camera::ui_camera(string name) {
    this->name = name;
    type = "NOT_OBJ";
}

void ui_camera::show_details() {
    if (this->open) {
        ImGui::InputFloat3("Position", glm::value_ptr(cam->position));
        ImGui::InputFloat3("Looking at", glm::value_ptr(cam->lookAt));
        ImGui::InputFloat3("vup", glm::value_ptr(cam->up));
        ImGui::SliderFloat("Fov", &cam->fov, 20.f, 110.f);
    }
}

//std::shared_ptr<ui_object> ui_camera::clone() const {
//    return std::make_shared<ui_camera>(*this);
//}

std::string ui_camera::save() {
    return "";
}

float ui_camera::get_camDistance() {
	float distance[3];
    float result = 0;

    for (int i = 0; i < 3; i++) {
        distance[i] = lookat[i] - position[i];

		result += distance[i] * distance[i];
    }

	return sqrt(result);
};

//Cube
ui_cube::ui_cube(string name) {
	this->name = name;
    type = "i cube";
}

ui_cube::ui_cube(ObjectInsertSet insertSet, string name) {
    this->name = name;
    type = "i cube";

    for (int i = 0; i < 3; i++) {
        this->position[i] = insertSet.position[i];
        this->scale[i] = insertSet.scale[i];
        this->rotation[i] = insertSet.rotation[i];
        if (insertSet.color) {
            this->color[i] = (*insertSet.color)[i];
        }
    }
}

//std::shared_ptr<ui_object> ui_cube::clone() const {
//    return std::make_shared<ui_cube>(*this);
//}

std::string ui_cube::save() {
    std::string settings = "";

    for (int i = 0; i < 3; i++) {
        settings += std::to_string(position[i]) + " ";
    }
    settings += "\n";
    for (int i = 0; i < 3; i++) {
        settings += std::to_string(rotation[i]) + " ";
    }
    settings += "\n";
    for (int i = 0; i < 3; i++) {
        settings += std::to_string(scale[i]) + " ";
    }
    settings += "\n";
    for (int i = 0; i < 3; i++) {
        settings += std::to_string(color[i]) + " ";
    }
    settings += "\n";

    return settings;
}

//Sphere
ui_sphere::ui_sphere(string name) {
    this->name = name;
    type = "i sphere";
}

ui_sphere::ui_sphere(ObjectInsertSet insertSet, string name) {
    this->name = name;
    type = "i sphere";

    for (int i = 0; i < 3; i++) {
        this->position[i] = insertSet.position[i];
        this->scale[i] = insertSet.scale[i];
        this->rotation[i] = insertSet.rotation[i];
        if (insertSet.color) {
            this->color[i] = (*insertSet.color)[i];
        }
    }
}

std::string ui_sphere::save() {
    std::string settings = "";

    for (int i = 0; i < 3; i++) {
        settings += std::to_string(position[i]) + " ";
    }
    settings += "\n";
    for (int i = 0; i < 3; i++) {
        settings += std::to_string(rotation[i]) + " ";
    }
    settings += "\n";
    for (int i = 0; i < 3; i++) {
        settings += std::to_string(scale[i]) + " ";
    }
    settings += "\n";
    for (int i = 0; i < 3; i++) {
        settings += std::to_string(color[i]) + " ";
    }
    settings += "\n";

    return settings;
}

//std::shared_ptr<ui_object> ui_sphere::clone() const {
//    return std::make_shared<ui_sphere>(*this);
//}

//Plane
ui_plane::ui_plane(std::string name) {
    this->name = name;
    type = "i quad";
}

ui_plane::ui_plane(ObjectInsertSet insertSet, string name) {
    this->name = name;
    type = "i quad";

    for (int i = 0; i < 3; i++) {
        this->position[i] = insertSet.position[i];
        this->scale[i] = insertSet.scale[i];
        this->rotation[i] = insertSet.rotation[i];
        if (insertSet.color) {
            this->color[i] = (*insertSet.color)[i];
        }
    }
}

std::string ui_plane::save() {
    std::string settings = "";

    for (int i = 0; i < 3; i++) {
        settings += std::to_string(position[i]) + " ";
    }
    settings += "\n";
    for (int i = 0; i < 3; i++) {
        settings += std::to_string(rotation[i]) + " ";
    }
    settings += "\n";
    for (int i = 0; i < 3; i++) {
        settings += std::to_string(scale[i]) + " ";
    }
    settings += "\n";
    for (int i = 0; i < 3; i++) {
        settings += std::to_string(color[i]) + " ";
    }
    settings += "\n";

    return settings;
}

//std::shared_ptr<ui_object> ui_plane::clone() const {
//    return std::make_shared<ui_plane>(*this);
//}

//OBJ File
ui_obj_file::ui_obj_file(std::string name) {
    this->name = name;
    type = "i obj";
}

std::string ui_obj_file::save() {
    return "";
}

//std::shared_ptr<ui_object> ui_obj_file::clone() const {
//    return std::make_shared<ui_obj_file>(*this);
//}