#pragma once
#include "buffer.h"
#include <shape/mesh.h>
#include "descriptor.h"


// cet class marche sur le concept d'etre sur un thread pour le command pool

#define DEFAULT_FENCE_TIMEOUT 100000000000


const vector<const char*> deviceExtensions = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME,
	VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME,
	VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME,
	VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME,
	VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME,
	VK_KHR_SPIRV_1_4_EXTENSION_NAME,
	VK_KHR_SHADER_FLOAT_CONTROLS_EXTENSION_NAME,
	VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME
};

/**
 * @brief 
 * Le strut Feature garde les feautures Vulkan qu'on veut activé.
 */
struct Feature {

	VkPhysicalDeviceBufferDeviceAddressFeatures bufferAddressFeatures = { .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES,
		.bufferDeviceAddress = VK_TRUE
	};

	VkPhysicalDeviceAccelerationStructureFeaturesKHR accelFeatures = {.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR,
		.accelerationStructure = VK_TRUE
	};

	VkPhysicalDeviceRayTracingPipelineFeaturesKHR rtFeatures = { .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR,
		.rayTracingPipeline = VK_TRUE
	};

	VkPhysicalDeviceFeatures2 features2{
		.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
		.pNext = NULL
	};
	VkPhysicalDeviceDescriptorIndexingFeatures indexing{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES,
		.descriptorBindingPartiallyBound = VK_TRUE,
		.descriptorBindingVariableDescriptorCount = VK_TRUE,
		.runtimeDescriptorArray = VK_TRUE
	};

	vector<VkBaseOutStructure*> featureChain;

	void connect();
	
};

/**
 * @brief 
 * Le struct RayTracingProcs garde
 *  les fonctions du ray tracing pipeline et de l'accélération structure.
 */
struct RayTracingProcs {
	PFN_vkCreateAccelerationStructureKHR        vkCreateAccelerationStructureKHR = nullptr;
	PFN_vkDestroyAccelerationStructureKHR       vkDestroyAccelerationStructureKHR = nullptr;
	PFN_vkGetAccelerationStructureBuildSizesKHR vkGetAccelerationStructureBuildSizesKHR = nullptr;
	PFN_vkGetAccelerationStructureDeviceAddressKHR vkGetAccelerationStructureDeviceAddressKHR = nullptr;
	PFN_vkCmdBuildAccelerationStructuresKHR     vkCmdBuildAccelerationStructuresKHR = nullptr;
	PFN_vkCreateRayTracingPipelinesKHR          vkCreateRayTracingPipelinesKHR = nullptr;
	PFN_vkGetRayTracingShaderGroupHandlesKHR    vkGetRayTracingShaderGroupHandlesKHR = nullptr;
	PFN_vkCmdTraceRaysKHR                       vkCmdTraceRaysKHR = nullptr;
};


/**
 * @brief 
 * La class VkRessource permet de créer et de donner accèes à des ressources Vulkan
 */
class vkRessource {

	unique_ptr<instance> instance_ptr;
	unique_ptr<physicalDevice> pdevice_ptr;
	unique_ptr<surface> surface_ptr;
	unique_ptr<device> device_ptr;
	unique_ptr<descriptor> setDescriptor;
	VkCommandPool pool;

	vector<void*> featureChain; 

	void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory) const;
	void createCommandPool();
	void linkerFonction();
	void cleanup();

	public:

		RayTracingProcs rt;
		vkRessource(GLFWwindow* w);
		vkRessource(device*, physicalDevice*, surface*);
		~vkRessource();

		VkCommandBuffer createTmpCmdBuffer(VkCommandBufferUsageFlags flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, VkCommandBufferInheritanceInfo* info = nullptr) const;
		unique_ptr<buffer> createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties) const;
	
		unique_ptr<buffer> createVertexBuffer(const vector<glm::vec3>,bool ray = true);
		unique_ptr<buffer> createVertexBuffer(const vector<vertex>,bool ray = true);
		unique_ptr<buffer> createIndexBuffer(const vector<uint32_t>,bool ray = true);
		
		const VkDevice getDevice() const;

		const VkQueue getGraphicQueue() const;
		const VkQueue getPresentQueue() const;

		const VkCommandPool getCommandPool() const;
		const VkPhysicalDevice getPhysicalDevice() const;
		const VkSurfaceKHR getSurface() const;
		const VkInstance getInstance() const;
		VkDescriptorSet getSet(uint32_t index,uint32_t offset = 0) const;

		uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) const;
		void flushCommandBuffer(VkCommandBuffer commandBuffer, bool free = true);
		void staggingBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkBuffer, void*);
		void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) const;
		VkDeviceAddress queryBufferAddress(VkBuffer) const;
		VkDeviceAddress queryASAddress(VkAccelerationStructureKHR ) const;
		void setImageLayout(
			VkCommandBuffer cmdbuffer,
			VkImage image,
			VkImageLayout oldImageLayout,
			VkImageLayout newImageLayout,
			VkImageSubresourceRange subresourceRange,
			VkPipelineStageFlags srcStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
			VkPipelineStageFlags dstStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT
		);
		
		void createDescriptorPool(vector<VkDescriptorPoolSize> poolSizes);
		void addDescriptorSetLayout(vector<VkDescriptorType> type, vector<VkShaderStageFlags> flags, uint32_t sizeDescriptor = 1);
		void addPushConstant(VkShaderStageFlags flag, uint32_t size);
		void allocateDescriptorSet();
		const vector<VkDescriptorSetLayout>& getDescriptorSetLayout() const;
		const vector<VkPushConstantRange>& getPushConstant() const;
		vkRessource(const vkRessource&) = delete;
		vkRessource& operator=(const vkRessource&) = delete;
};