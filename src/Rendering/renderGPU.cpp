#include "renderGPU.h"

void renderRayTracingVulkan::initWindow()
{
	glfwInit();

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	

	window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
}

void renderRayTracingVulkan::initPipeline()
{
	shader raygen(vk.get(), SHADER_DIR"raygen.rgen.spv", VK_SHADER_STAGE_RAYGEN_BIT_KHR, VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR, 0);
	shader miss(vk.get(), SHADER_DIR"miss.rmiss.spv", VK_SHADER_STAGE_MISS_BIT_KHR, VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR, 1);
	shader shadow(vk.get(), SHADER_DIR"shadow.rmiss.spv", VK_SHADER_STAGE_MISS_BIT_KHR, VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR, 2);
	shader closesthit(vk.get(), SHADER_DIR"closesthit.rchit.spv", VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR, VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR, 3);
	shader ambienthit(vk.get(), SHADER_DIR"ambientOcclusion.rchit.spv", VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR, VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR, 4);


	pipeline->addShader(&raygen);
	pipeline->addShader(&miss);
	pipeline->addShader(&shadow);
	pipeline->addShader(&closesthit);
	pipeline->addShader(&ambienthit);

	cam.position = glm::vec4(0.0f, 0.0f, -10.0f, 1.0f); 
	cam.lookAt = glm::vec4(0.0f,  0.0f, 0.0f, 1.0f); 
	cam.up = glm::vec4(0.0f, 1.0f, 0.0f, 0.0f);
	cam.fov = 60;
	pipeline->setCamera(&cam);
	pipeline->createPipeline();
}

void renderRayTracingVulkan::initDescriptor()
{

		vector<VkDescriptorType> typeSet = {
			VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR,
			VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
			VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
			VK_DESCRIPTOR_TYPE_STORAGE_IMAGE
		};

		vector<VkShaderStageFlags> flagSet = {
			 VK_SHADER_STAGE_RAYGEN_BIT_KHR | VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR,
			 VK_SHADER_STAGE_RAYGEN_BIT_KHR | VK_SHADER_STAGE_COMPUTE_BIT,
			 VK_SHADER_STAGE_COMPUTE_BIT,
			 VK_SHADER_STAGE_COMPUTE_BIT
		};

		vk->addDescriptorSetLayout(typeSet, flagSet);

		vector<VkDescriptorType> typeSet1 = {
			VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
			VK_DESCRIPTOR_TYPE_STORAGE_BUFFER
		};

		vector<VkShaderStageFlags> flagSet1 = {
			 VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR,
			 VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR
		};
		vk->addDescriptorSetLayout(typeSet1, flagSet1,MAX_MESHES);

		vector<VkDescriptorType> typeSet2 = {
			VK_DESCRIPTOR_TYPE_STORAGE_BUFFER
			
		};

		vector<VkShaderStageFlags> flagSet2 = {
			 VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR,
		};
		vk->addDescriptorSetLayout(typeSet2, flagSet2, MAX_MESHES);

		

		vk->addPushConstant(VK_SHADER_STAGE_RAYGEN_BIT_KHR | VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR,sizeof(Camera));


		vector<VkDescriptorPoolSize> poolSizes = {
			{ VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, MAX_FRAMES_IN_FLIGHT },
			{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, MAX_FRAMES_IN_FLIGHT },
			{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, MAX_FRAMES_IN_FLIGHT },
			{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, MAX_FRAMES_IN_FLIGHT },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, MAX_MESHES * MAX_FRAMES_IN_FLIGHT },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, MAX_MESHES * MAX_FRAMES_IN_FLIGHT },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,  MAX_MESHES * MAX_FRAMES_IN_FLIGHT }
		};
		vk->createDescriptorPool(poolSizes);
		vk->allocateDescriptorSet();
}


void renderRayTracingVulkan::mainLopp()
{
	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();
		imgui->newFrame();
		imgui->ui_build();

		pipeline->drawFrame();

	}
	VK_CHECK(vkDeviceWaitIdle(vk->getDevice()));
}

void renderRayTracingVulkan::cleanup()
{

	glfwDestroyWindow(window);
	glfwTerminate();
}

renderRayTracingVulkan::renderRayTracingVulkan()
{
	initWindow();
	vk = make_unique<vkRessource>(window);
	initDescriptor();
	display_ptr = make_unique<display>(window, vk.get());
	acc = make_unique<acceleration>(vk.get());
	imgui = make_unique<ui>(vk.get(), window,display_ptr.get(),&cam,acc.get());
	pipeline = make_unique<raytracingPipeline>(vk.get(), display_ptr.get());

	initPipeline();
}

renderRayTracingVulkan::~renderRayTracingVulkan()
{
	cleanup();
}


void renderRayTracingVulkan::run()
{
	mainLopp();
}
