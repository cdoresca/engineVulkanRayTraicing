#pragma once
#include "display.h"
#include "Rendering/camera.h"
#include "shader.h"


const uint32_t  localX = 16;
const uint32_t  localY = 16;

/**
 * @brief 
 * La class pipeline est class abstraite pour les pipelines Vulkan.
 */
class pipeline {

	protected:
		vector<VkCommandBuffer> cmd;
		vkRessource* vk;
		display* display_ptr;
		VkPipelineLayout pipelineLayout;
		VkPipeline graphicsPipeline;
		
		vector<VkSemaphore> imageAvailableSemaphores;
		vector<VkSemaphore> renderFinishedSemaphores;
		vector<VkFence> inFlightFences;

		uint32_t currentFrame = 0;
		virtual void createPipeline() = 0;
		

		
		virtual void recordCommand(VkCommandBuffer commandBuffer, uint32_t imageIndex, uint32_t frameIndex) = 0;

		void createSyncObjects();
		void createCommandBuffers();

		virtual void cleanup() = 0;

	public:

		pipeline(vkRessource*, display*);
		~pipeline();

		virtual void drawFrame() = 0;
		const VkPipelineLayout& getPipelineLayout();
};

/**
 * @brief 
 * La class rasterizationPipeline est enfant de pipeline et représente le pipeline de rasterization.
 */

class rasterizationPipeline:pipeline {

	void createPipeline() override;
	
	void recordCommand(VkCommandBuffer commandBuffer, uint32_t imageIndex, uint32_t frameIndex) override;
	void cleanup() override;

	public :
		rasterizationPipeline(vkRessource*, display*);
		~rasterizationPipeline();
		void drawFrame() override;
};

/**
 * @brief 
 * La class raytracingPipeline est enfant de pipeline et représente le pipeline de ray tracing.
 */
class raytracingPipeline:public pipeline {

	vector<VkRayTracingShaderGroupCreateInfoKHR> shaderGroups;
	vector<VkPipelineShaderStageCreateInfo> shaderStages;
	vector<uint8_t> shaderHandles;

	unique_ptr<buffer> shaderBindingTable;

	VkStridedDeviceAddressRegionKHR raygenRegion{};    
	VkStridedDeviceAddressRegionKHR missRegion{};     
	VkStridedDeviceAddressRegionKHR hitRegion{};       
	VkStridedDeviceAddressRegionKHR callableRegion{};

	VkPipeline computePipeline;

	Camera* cam;

	void createShaderBindingTable(const VkRayTracingPipelineCreateInfoKHR& rtPipelineInfo);
	void recordCommand(VkCommandBuffer commandBuffer, uint32_t imageIndex, uint32_t frameIndex) override;
	void cleanup() override;
	

	public :
		raytracingPipeline(vkRessource*, display*);
		~raytracingPipeline(); 
		
		void createPipeline() override;
		void setCamera(Camera* cam);
		void drawFrame() override;
		void addShader(shader*);
};


