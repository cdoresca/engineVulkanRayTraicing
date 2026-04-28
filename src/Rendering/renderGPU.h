#pragma once

#include "vulkanWrapper/display.h"
#include "vulkanWrapper/accelerationStructure.h"
#include "vulkanWrapper/pipeline.h"
#include "UI/UI.h"


/**
 * @brief 
 * La class renderRayTracingVulkan coeur du code Vulkan.
 */
class renderRayTracingVulkan {

	GLFWwindow* window;

	unique_ptr<vkRessource> vk;

	unique_ptr<display> display_ptr;
	unique_ptr<ui> imgui;
	unique_ptr<raytracingPipeline> pipeline;
	unique_ptr<acceleration> acc;
	Camera cam{};

	void initWindow();
	void initPipeline();
	void initDescriptor();
	
	void mainLopp();
	void cleanup();

	public:
		renderRayTracingVulkan();
		~renderRayTracingVulkan();
		void run();
};