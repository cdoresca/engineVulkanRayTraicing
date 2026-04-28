#pragma once

#include <set>

#include "instance.h"
#include "queueFamily.h"
#include "surface.h"
#include "swapChainSupportDetail.h"

/**
 * @brief 
 * La class physicalDevice gère le VkPhysicalDevice. On choisit quel processeur graphique qui est 
 * disponible selon les exentsions demandées.
 * 
 */
class physicalDevice {

	const int type;

	VkPhysicalDevice device;
	VkPhysicalDeviceFeatures2 feature2;
	VkPhysicalDeviceFeatures feature{};

	VkBaseOutStructure* f;

	surface* surface_ptr;
	instance* instance_ptr;

	vector<const char*> deviceExtensions;
	vector<VkBaseOutStructure*> deviceFeature;
	
	bool isDeviceSuitable(const VkPhysicalDevice);
	bool isQueueSuitable(const VkPhysicalDevice) const;
	bool isSwapChainSuitable(const VkPhysicalDevice) const;
	bool isExtensionSuitable(const VkPhysicalDevice) const;
	bool isFeatureSuitable(const VkPhysicalDevice);

	void pickPhysicalDevice();
	void moveFrom(physicalDevice&);
	void cleanup();


	public :

		physicalDevice(instance*,const int,surface*, const vector<const char*>&, VkPhysicalDeviceFeatures2&, const vector<VkBaseOutStructure*>&);
		~physicalDevice();

		physicalDevice(const physicalDevice&) = delete;
		physicalDevice& operator=(const physicalDevice&) = delete;

		physicalDevice(physicalDevice&&) noexcept;
		physicalDevice& operator=(physicalDevice&&) noexcept;

		
		const VkPhysicalDevice get() const;
		const VkBaseOutStructure* getFeature() const;
		const vector<const char*>& getExtension() const;
};