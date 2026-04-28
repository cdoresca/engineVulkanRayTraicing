#pragma once

#include <vulkan/vulkan.h>
#include <glfw3.h>

#include<vector>

#include <stdexcept>
#include <fstream>
#include <iostream>

using namespace std;

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

#define VK_CHECK(x)                                                 \
	do                                                              \
	{                                                               \
		VkResult err = x;                                           \
		if (err)                                                    \
		{                                                           \
			std::cout <<"Detected Vulkan error: " << err << std::endl; \
			abort();                                                \
		}                                                           \
	} while (0)


const vector<const char*> validationLayers = {
	"VK_LAYER_KHRONOS_validation"

};
const vector<VkValidationFeatureEnableEXT> enables = {
	VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_EXT,
	VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_RESERVE_BINDING_SLOT_EXT
};

/**
 * @brief 
 * La class instance gère le VkInstance et VkDebugUtilsMessengerEXT(Validation Layers).
 *
 */

class instance {

	VkInstance m_instance;
	VkDebugUtilsMessengerEXT debugMessenger;

	bool enableRayTracing;

	void createInstance();
	void createAppInfo(VkApplicationInfo&) const;
	void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
	void setupDebugMessenger();
	void cleanup();
	void moveFrom(instance&);


	vector<const char*> getRequiredExtensions() const;

	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData);

	public:
		instance(bool ray = true);
		~instance();

		instance(const instance&) = delete;
		instance& operator=(const instance&) = delete;

		instance(instance&& other) noexcept;
		instance& operator=(instance&& other) noexcept;

		const VkInstance get() const;
		const VkDebugUtilsMessengerEXT getDebug() const;

};

VkResult CreateDebugUtilsMessengerEXT(VkInstance instance,
	const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
	const VkAllocationCallbacks* pAllocator,
	VkDebugUtilsMessengerEXT* pDebugMessenger);

void DestroyDebugUtilsMessengerEXT(VkInstance instance,
	VkDebugUtilsMessengerEXT debugMessenger,
	const VkAllocationCallbacks* pAllocator);