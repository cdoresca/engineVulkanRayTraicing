// Important note to the reader who wish to integrate imgui_impl_vulkan.cpp/.h in their own engine/app.
// - Common ImGui_ImplVulkan_XXX functions and structures are used to interface with imgui_impl_vulkan.cpp/.h.
//   You will use those if you want to use this rendering backend in your engine/app.
// - Helper ImGui_ImplVulkanH_XXX functions and structures are only used by this example (main.cpp) and by
//   the backend itself (imgui_impl_vulkan.cpp), but should PROBABLY NOT be used by your own engine/app code.
// Read comments in imgui_impl_vulkan.h.

#include "imgui.h"
#include <imgui_impl_glfw.h>
#define VK_USE_PLATFORM_WIN32_KHR
#include "imgui_impl_vulkan.h"
#include <windows.h>
#include <stdio.h>          // printf, fprintf
#include <stdlib.h>         // abort
#include <tchar.h>

#include "UI/object.h"
#include "world.h"
#include "Rendering/camera.h"
#include "UI.h"

#include "UI/save_loader.h"
#include <chrono>
#include <thread>
#include <atomic>


#include "ImGuizmo.h"
#include "UI_guizmo.h"
#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui_internal.h"
#include "ImSequencer.h"
#include "ImZoomSlider.h"
#include "ImCurveEdit.h"
#include "GraphEditor.h"
#include <math.h>
#include <vector>
#include <algorithm>
#include <glm/gtc/type_ptr.hpp>
#include <VulkanWrapper/buffer.h>


//Function used to show elapsed time during rendering. Also gives total time at the end
static void show_time(double elapsed_ms) {
    auto total_ms = static_cast<long long>(elapsed_ms);

    long long hours = total_ms / 3'600'000;
    long long minutes = (total_ms / 60'000) % 60;
    long long seconds = (total_ms / 1'000) % 60;
    long long milliseconds = total_ms % 1'000;

    ImGui::Text("Task running for: %01lldh:%02lldm:%02llds:%03lldms", hours, minutes, seconds, milliseconds);
}

//Thread that starts the rendering process
void thread_render(settings set, std::atomic<bool>& done, std::atomic<int>& rows_done, vector<mesh> meshes, vector<mat> mats) {
    
    for(int i = 0; i<meshes.size(); i++){
        for(int j = 0; j < meshes[i].vertices.size(); j++){
            meshes[i].vertices[j].pos = getMatrixParent(meshes[i].him) * meshes[i].him->model * glm::vec4(meshes[i].vertices[j].pos, 1.0f);
            
        }
    }
    
    world w;
    w.build(set);
    w.add_shapes(meshes,mats);
    w.render(rows_done);

    done = true;
}

static void check_vk_result(VkResult err)
{
    if (err == VK_SUCCESS)
        return;
    fprintf(stderr, "[vulkan] Error: VkResult = %d\n", err);
    if (err < 0)
        abort();
}

// Main code
int ui::ui_build()
{
    //Gizmo's scene
    drawScene();
    
    //Menu Bar
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Load")) {
                load_settings(set);
                for (int i = 0; i < 3; i++) {
                    cam.lookat[i] = set.lookat[i];
                    cam.position[i] = set.position[i];
                    cam.vup[i] = set.vup[i];
                }

                new_set = load_objects(objects);
                std::vector<ui_object*> new_objects;
                for (shared_ptr<ui_object> obj : new_set) {
                    new_objects.push_back(obj.get());
                }
                objects = new_objects;
            }

            if (ImGui::MenuItem("Save")) {
                for (int i = 0; i < 3; i++) {
                    set.lookat[i] = cam.lookat[i];
                    set.position[i] = cam.position[i];
                    set.vup[i] = cam.vup[i];
                }

                save_settings(set); //Saves settings as JSON
                save_objects(objects); //saves scene as obj
            }

            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Add")) {
            if (ImGui::MenuItem("Scene Item")) {
                objCreationMenuOpen = !objCreationMenuOpen;
            }

            if (ImGui::MenuItem("OBJ Scene Item")) {
                objFileExplorer.Open();
            }

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Misc")) {
            if (ImGui::MenuItem("Options")) {
                //TODO make actual options screen with options we might want
                optionMenuOpen = !optionMenuOpen;
            }

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Shader")) {
            if (ImGui::MenuItem("Lambertian")) {
                camera->shader = 0;
            }
            if (ImGui::MenuItem("Ambient Occlusion")) {
                camera->shader = 1;
            }
            ImGui::EndMenu();
        }

        menuBarSize = ImGui::GetWindowSize().y;
        ImGui::EndMainMenuBar();

    }

    /*
    File explorer for .obj format files
    This section des two things.
    -Adds the ui version of the object to the ui objects list
    -Adds it to the meshes for Vulkan
    */
    objFileExplorer.Display();
    if (objFileExplorer.HasSelected())
    {
        vector<mesh> meshes;
        objFilePath = objFileExplorer.GetSelected().string();
        objFileName = objFileExplorer.GetSelected().filename().string();
        objFileExplorer.ClearSelected();
        addObjectAccelerator(objFilePath, objFileName);
       
    }

    //Objects Window
    ImGui::SetNextWindowSize(ImVec2(io->DisplaySize.x / 5, io->DisplaySize.y - menuBarSize));
    ImGui::SetNextWindowPos(ImVec2(0, menuBarSize));
    ImGui::Begin("Objects", &objOpen, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize);

    for (auto& obj : objects) {
        ui::DrawObjectNode(*obj, objects);
    }

    ImGui::End();

    //Details Window
    ImGui::SetNextWindowSize(ImVec2(io->DisplaySize.x / 5, io->DisplaySize.y - menuBarSize));
    ImGui::SetNextWindowPos(ImVec2(4 * io->DisplaySize.x / 5, menuBarSize));
    ImGui::Begin("Details", &objOpen, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize);

    for (auto& obj : objects) {
        obj->show_details();
    }

    ImGui::SetCursorPos(ImVec2(10, ImGui::GetWindowSize().y - 70));
    if (ImGui::Button("Render", ImVec2(ImGui::GetWindowSize().x - 20, 50)))
        renderOpen = !renderOpen;

    ImGui::End();

    //Render Settings Window
    if (renderOpen) {
        ImGui::SetNextWindowSize(ImVec2(600, 400)); //TODO
        ImGui::Begin("Render Settings", &renderOpen, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize);
        ImGui::InputText("filename", set.name, IM_ARRAYSIZE(set.name));
        ImGui::InputInt("Image Width", &set.imgWidth);
        ImGui::InputInt("Samples per pixel", &set.samplesPerPixel);
        ImGui::InputInt("Max Depth", &set.maxDepth);
        ImGui::InputInt("Defocus Angle", &set.defocusAngle);
        
        if (ImGui::BeginCombo("Aspect ratio", currentLabel)) {
            if (ImGui::Selectable("16:9")) {
                set.aspectRatio = 16.0 / 9.0;
                currentLabel = "16:9";
            }
            if (ImGui::Selectable("1.85:1")) { 
                set.aspectRatio = 1.85 / 1.0; 
                currentLabel = "1.85:1";
            }
            if (ImGui::Selectable("2.39:1")) 
            {
                set.aspectRatio = 2.39 / 1.0;
                currentLabel = "2.39:1";
            }
            if (ImGui::Selectable("4:3")) 
            {
                set.aspectRatio = 4.0 / 3.0;
                currentLabel = "4:3";
            }
            if (ImGui::Selectable("1:1"))
            {
                set.aspectRatio = 1.0 / 1.0;
                currentLabel = "1:1";
            }
            ImGui::EndCombo();
        }

        ImGui::SetCursorPos(ImVec2(10, ImGui::GetWindowSize().y - 70));
        if (ImGui::Button("Go!", ImVec2(ImGui::GetWindowSize().x - 20, 50))) {
            renderOpen = !renderOpen;
            renderDone = false;
            renderingOpen = true;

            //Start Rendering
           
            for (int i = 0; i < 3; i++) {
                set.lookat[i] = camera->lookAt[i];
                set.position[i] = camera->position[i];
                set.vup[i] = camera->up[i];
            }
            set.vfov = camera->fov;

            //Waits for previous render to finish
            joinThread();

            //Lets the output visualization reload
            reload = true;

            //Starts the render
            thread1 = std::thread(thread_render, set, std::ref(renderDone), std::ref(rows_done), accel->getScene(), mats);
            c_start = std::clock();
        }

        ImGui::End();
    }

    //Rendering progress tracking window 
    if (renderingOpen) {
        ImGui::SetNextWindowSize(ImVec2(io->DisplaySize.x / 4, io->DisplaySize.y / 4));
        ImGui::Begin("Rendering Image...", &renderingOpen, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize);

        show_time(1000.0 * (c_end - c_start) / CLOCKS_PER_SEC);

        if (renderDone) {
            ImGui::Text("Done!");
            showImgOpen = true;
        }
        else {
            c_end = std::clock();

            ImGui::Text("Task separated on: %d threads", std::thread::hardware_concurrency() - 1);
            rows = rows_done;
            ImGui::Text("Scanlines remaining: %d", rows);
        }

        ImGui::End();
    }

    //Menu window to create an object
    if (objCreationMenuOpen) {
        ImGui::Begin("New Item", &objCreationMenuOpen, ImGuiWindowFlags_NoCollapse);

        ImGui::InputText("Name", m_buffer, IM_ARRAYSIZE(m_buffer));
        ImGui::Combo("Type", &dropdownIndex, dropdown, IM_ARRAYSIZE(dropdown));
        if (name_exists) {
            ImGui::Text("You cannot use a name already in use!");
        }
        if (ImGui::Button("Create", ImVec2(ImGui::GetWindowSize().x - 20, 30))) {
            name_exists = false;

            for (auto object : objects) {
                //Can't have two items with same name (The names are used as ID by ImGui)
                if (object->name == m_buffer) {
                    name_exists = true;
                }
            }
            if (!name_exists) {
                if (dropdownIndex == 0) { //Cube
                    ui_cube* new_obj = new ui_cube(m_buffer);
                    objects.push_back(new_obj);
                    addObjectAccelerator(OBJ_DIR"cube.obj", new_obj->name);
                }
                else if (dropdownIndex == 1) { //Sphere
                    ui_sphere* new_obj = new ui_sphere(m_buffer);
                    objects.push_back(new_obj);
                    addObjectAccelerator(OBJ_DIR"sphere.obj", new_obj->name);
                }
                else if (dropdownIndex == 2) { //Plane
                    ui_plane* new_obj = new ui_plane(m_buffer);
                    objects.push_back(new_obj);
                    addObjectAccelerator(OBJ_DIR"plane.obj", new_obj->name);
                }

                objCreationMenuOpen = false;
            }
        }

        ImGui::End();
    }

    //Options Menu
    if (optionMenuOpen) {
        ImGui::Begin("Options", &optionMenuOpen, ImGuiWindowFlags_NoCollapse);

        ImGui::ColorEdit3("Background color", (float*)&background_color);

        ImGui::End();
    }
    
    moveCamera();

    return 0;
}

void ui::cleanup()
{
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    vkDestroyDescriptorPool(vk->getDevice(), pool, nullptr);
    joinThread();
}

void ui::joinThread()
{
    //Make sure render ends
    if (thread1.joinable()) {
        thread1.join();
    }
}

void ui::addObjectAccelerator(string path, string name)
{
    vector<mesh> meshes;
    if(!loadOBJ(path, meshes, mats)) return;

    node* head = new node();
    head->name = name;
    head->parent = &scene;
    head->model = glm::mat4(1.0f);
    scene.child.push_back(head);

    auto newObject = new ui_object(head->name, head, accel);
    objects.push_back(newObject);

    if (meshes.size() == 1) {
        mat base;
        base.Kd = vec3(1, 1, 1);
        base.Ke = vec3(0, 0, 0);
        mats.push_back(base);
        meshes[0].him = head;
        accel->add(meshes[0]);
    }
    else if (meshes.size() > 1) {

        for (int i = 0; i < meshes.size(); i++) {
            node* tmp = new node();
            tmp->name = meshes[i].name;
            tmp->parent = head;
            tmp->model = glm::mat4(1.0f);
            head->child.push_back(tmp);
            meshes[i].him = tmp;
            auto child = new ui_object(tmp->name, tmp, accel);
            newObject->children.push_back(child);
            accel->add(meshes[i]);
           
        }
    }
    addMaterial();
    accel->rebuildTlas(VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR);

}

void ui::addMaterial()
{
    for (int i = numMat; i < mats.size(); i++) {

        MatGpu m{};
        m.diffuse = glm::vec4(mats[i].Kd[0], mats[i].Kd[1], mats[i].Kd[2],0);
        float w = (mats[i].Ke[0] > 0.0f ||
            mats[i].Ke[1] > 0.0f ||
            mats[i].Ke[2] > 0.0f) ? 1.0f : 0.0f;
        m.emissive = glm::vec4(mats[i].Ke[0], mats[i].Ke[1], mats[i].Ke[2], w);

        matBuffer.push_back(vk->createBuffer(sizeof(MatGpu) ,
            VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
        ));
        vk->staggingBuffer(sizeof(MatGpu),
            VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            matBuffer.back()->get(),
            &m
        );
        updateDescriptorSetMaterial(matBuffer.size() - 1 , matBuffer.back().get());
    }
    numMat = mats.size();
}

void ui::updateDescriptorSetMaterial(uint32_t index, buffer* matBuffer)
{
    for (auto i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {


        VkDescriptorSet set1 = vk->getSet(i, 2);

        VkDescriptorBufferInfo materialDescriptor{};
        materialDescriptor.buffer = matBuffer->get();
        materialDescriptor.offset = 0;
        materialDescriptor.range = VK_WHOLE_SIZE;

        VkWriteDescriptorSet materialWrite{};
        materialWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        materialWrite.pNext = nullptr;
        materialWrite.dstSet = set1;
        materialWrite.dstBinding = 0;
        materialWrite.dstArrayElement = index;
        materialWrite.descriptorCount = 1;
        materialWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        materialWrite.pBufferInfo = &materialDescriptor;

        vkUpdateDescriptorSets(vk->getDevice(), 1, &materialWrite, 0, VK_NULL_HANDLE);
    }
}


ui::ui(vkRessource* v, GLFWwindow* w, display*d,Camera* c, acceleration* a):vk(v),cam("camera"),window(w),display_ptr(d),camera(c), accel(a)
{
    createDescriptorPool();
    createContext();
    objects.push_back(&cam); //Have to ensure all names are different
    cam.cam = camera;
    //Default settings
    set.aspectRatio = 16.0/9.0;
    set.imgWidth = 1920;
    set.samplesPerPixel = 1;
    set.maxDepth = 15;
    set.vfov = 40;
    set.defocusAngle = 0;

    scene.name = "scene";
    scene.parent = nullptr;
}

ui::~ui()
{
    cleanup();
}


// Helper functions

// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);



void ui::createDescriptorPool()
{
    VkDescriptorPoolSize poolSizes[] =
    {
        { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, IMGUI_IMPL_VULKAN_MINIMUM_IMAGE_SAMPLER_POOL_SIZE },
    };
    VkDescriptorPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    poolInfo.maxSets = 0;
    for (VkDescriptorPoolSize& pool_size : poolSizes)
        poolInfo.maxSets += pool_size.descriptorCount;
    poolInfo.poolSizeCount = (uint32_t)IM_ARRAYSIZE(poolSizes);
    poolInfo.pPoolSizes = poolSizes;
    vkCreateDescriptorPool(vk->getDevice(), &poolInfo, nullptr, &pool);
}

void ui::createContext()
{

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    io = &ImGui::GetIO(); 
    (void)io;
    io->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io->ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsLight();
    std::string path = std::string(FRONT_DIR) + "maple-mono-latin-400-normal.ttf";
    main_font = io->Fonts->AddFontFromFileTTF(path.c_str());
    ImGui_ImplGlfw_InitForVulkan(window, true);
    
    ImGui_ImplVulkan_InitInfo init_info = {};
    init_info.ApiVersion = VK_API_VERSION_1_4;              // Pass in your value of VkApplicationInfo::apiVersion, otherwise will default to header version.
    init_info.Instance = vk->getInstance();
    init_info.PhysicalDevice = vk->getPhysicalDevice();
    init_info.Device = vk->getDevice();
    init_info.QueueFamily = findQueueFamilies(vk->getPhysicalDevice(),vk->getSurface()).graphicFamily.value();
    init_info.Queue = vk->getGraphicQueue();
    init_info.DescriptorPool = pool;
    init_info.MinImageCount = display_ptr->getMinImageCount();
    init_info.ImageCount = display_ptr->getImageCount();
    init_info.PipelineInfoMain.RenderPass = display_ptr->getRenderPass();
    init_info.PipelineInfoMain.Subpass = 0;
    init_info.PipelineInfoMain.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
    
    ImGui_ImplVulkan_Init(&init_info);

}

void ui::newFrame()
{
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame(); // ou Win32
    ImGui::NewFrame();
}

void ui::addObject(ui_object* o)
{
    objects.push_back(o);
}

void ui::createFileBrowser()
{
    //Obj file
    objFileExplorer.SetTitle("Searching for .obj...");
    objFileExplorer.SetTypeFilters({ ".obj" });

    //JSON file
    JSONFileExplorer.SetTitle("Searching for JSON...");
    JSONFileExplorer.SetTypeFilters({ ".json" });
}

//Function to draw the tree of objects in the scene
void ui::DrawObjectNode(ui_object& obj, std::vector<ui_object*> objects, int depth, bool visible) {
    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_SpanFullWidth;
    if (obj.children.empty())
        flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
    if (obj.open)
        flags |= ImGuiTreeNodeFlags_Selected;

    // Visibility toggle before the node label (TODO not implement yet to have an effect on the scene)
    ImGui::PushID(obj.name.c_str());
    
    if (ImGui::Checkbox("##vis", &obj.visible))
    {
        obj.show();
        visible =!obj.visible;
        for (auto& child : obj.children) {
            child->visible = obj.visible;
              
        }
    }
    ImGui::SameLine();

    bool open = ImGui::TreeNodeEx(obj.name.c_str(), flags);
    if (ImGui::IsItemClicked()) {
        for (auto& obj : objects) {
			obj->open = false;
        }
        obj.open = !obj.open;
    };

    if (open && !obj.children.empty()) {
        for (auto& child : obj.children) {
            DrawObjectNode(*child, objects, depth + 1);
        }
    
        ImGui::TreePop();
    }
    ImGui::PopID();
}

void ui::moveCamera()
{
    if (objects[0]->visible) {
        captureMouse();
        captureKeyboard();
    }

}

void ui::captureMouse()
{
    if (ImGui::GetIO().WantCaptureMouse) return;
    if (ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
        ImVec2 mousePos = ImGui::GetMousePos();
        if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
            lastMousePos = mousePos;
        }
        float offsetX = mousePos.x - lastMousePos.x;
        float offsetY = lastMousePos.y - mousePos.y;

        lastMousePos = mousePos;

        offsetX *= sensitivity;
        offsetY *= sensitivity;

        yaw += offsetX;
        pitch += offsetY;

        if (pitch > 89.0f)
            pitch = 89.0f;
        if (pitch < -89.0f)
            pitch = -89.0f;

        glm::vec3 direction;
        direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        direction.y = sin(glm::radians(pitch));
        direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
        camera->lookAt = camera->position + glm::vec4(direction, 0);
    }
    
    float scroll = ImGui::GetIO().MouseWheel;
    camera->fov -= scroll;
}

void ui::captureKeyboard()
{

    if (ImGui::GetIO().WantCaptureKeyboard) return;

    glm::vec3 w = glm::vec3(glm::normalize(camera->lookAt - camera->position));
    glm::vec3 u = glm::normalize(glm::cross(glm::vec3(camera->up), w));
    glm::vec3 v = glm::cross(u, w);
  

    if (ImGui::IsKeyPressed(ImGuiKey_W) || ImGui::IsKeyPressed(ImGuiKey_UpArrow)) {
        camera->position += glm::vec4(w,0);
        camera->lookAt += glm::vec4(w, 0);
    }
    if (ImGui::IsKeyPressed(ImGuiKey_A) || ImGui::IsKeyPressed(ImGuiKey_LeftArrow)) {
        camera->position += glm::vec4(u, 0);
        camera->lookAt += glm::vec4(u, 0);
    }
    if (ImGui::IsKeyPressed(ImGuiKey_S) || ImGui::IsKeyPressed(ImGuiKey_DownArrow)) {
        camera->position -= glm::vec4(w, 0);
    }
    if (ImGui::IsKeyPressed(ImGuiKey_D) || ImGui::IsKeyPressed(ImGuiKey_RightArrow)) {
        camera->position -= glm::vec4(u, 0);
        camera->lookAt -= glm::vec4(u, 0);
    }
    if (ImGui::IsKeyPressed(ImGuiKey_E)) {
        camera->position += glm::vec4(v, 0);
        camera->lookAt += glm::vec4(v, 0);
    }
    if (ImGui::IsKeyPressed(ImGuiKey_F)) {
        camera->position -= glm::vec4(v, 0);
        camera->lookAt -= glm::vec4(v, 0);
    }
}
