#pragma once

#include "world.h"
#include "VulkanWrapper/vkRessource.h"
#include "VulkanWrapper/queueFamily.h"
#include "VulkanWrapper/display.h"
#include "imgui.h"
#include "object.h"
#include "imfilebrowser.h"
#include "Shape/obj_reader.h"
#include <chrono>
#include <thread>
#include <atomic>
#include <VulkanWrapper/accelerationStructure.h>


class ui {
protected:
    ImGuiIO* io;

    GLFWwindow* window;
    vkRessource* vk;
    display* display_ptr;

	VkDescriptorPool pool;
    acceleration* accel;
    

    Camera* camera;
    node  scene;
    vector<mat> mats;
    
    vector<unique_ptr<buffer>> matBuffer;
    // Load Font
    ImFont* main_font;

    // Our state
    ImVec4 background_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f); //Background color

    //Window states
    bool done = false;
    bool objOpen = true;
    bool detailsOpen = true;
    bool renderOpen = false;
    bool renderingOpen = false;
    bool objCreationMenuOpen = false;
    bool showImgOpen = false;
    bool optionMenuOpen = false;

    float yaw = -90.0f;
    float pitch = 0.0f;
    float distance = 5.0f;
    float sensitivity = 0.1f;
   
    bool firstMouse = true;

    ImVec2 lastMousePos;
    bool rotating = false;
    uint32_t numMat = 0;
    //Needed Vars
    settings set;
    const char* currentLabel = "16:9";
    //Create Object Menu
    char m_buffer[32] = "";
    const char* dropdown[4] = { "Cube", "Sphere", "Plane", "Light" };
    int dropdownIndex = 0;
    bool name_exists = false;

    //During Rendering
    std::thread thread1;
    std::atomic<bool> renderDone = false;
    int nb_scanlines = 0;
    std::clock_t c_start;
    std::clock_t c_end = std::clock();
    std::atomic<int> rows_done = 0;
    int rows = 0;

    //Other variables
    float menuBarSize = 0.0f;
    string objFilePath = "";
    string objFileName = "";

    //To avoid double rendering
	bool reload = false;

    //Objects initialization
    ui_camera cam{ "camera" };
    std::vector<ui_object*> objects;
    vector<shared_ptr<ui_object>> new_set; //Might be dead code


    ImGui::FileBrowser objFileExplorer;
    ImGui::FileBrowser JSONFileExplorer; //TODO save state

	void createDescriptorPool();
	void createContext();
    void addObject(ui_object*);
    void createFileBrowser();
    void cleanup();
    void joinThread();
    void addObjectAccelerator(string path, string name);
    void addMaterial();
    void updateDescriptorSetMaterial(uint32_t index, buffer* matBuffer);
    
    void DrawObjectNode(ui_object& obj, std::vector<ui_object*> objects, int depth = 0, bool visible = false);
    void moveCamera();
    void captureMouse();
    void captureKeyboard();

public :
	int ui_build();
    void drawScene();
	void newFrame();
    
    ui(vkRessource*, GLFWwindow*,display*,Camera* cam, acceleration* acc);
    ~ui();
    
};

static void thread_render(settings set, std::atomic<bool>& done, std::atomic<int>& rows_done,vector<mesh> meshes, vector<mat> mats);