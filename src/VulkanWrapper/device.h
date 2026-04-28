#pragma once

#include "physicalDevice.h"
/**
 * @brief 
 * La class gère le logical device, 
 * c'est-à-dire le VkDevice avec les VkQueue, present, graphic et compute.
 * Il active les features Vulkan. 
 * 
 */
class device {

	VkDevice m_device;
	VkQueue presentQueue;
	VkQueue graphicsQueue;
	VkQueue computeQueue;

	float priorityQueue = 1.0f;

	void createInfoQueue(vector<VkDeviceQueueCreateInfo>&) const;
	void createLogicalDevice();
	void CreateQueue(); 
	void cleanup();
	void moveFrom(device&);

	physicalDevice* physicalDevice_ptr;
	surface* surface_ptr;

	public:
		device(physicalDevice*,surface*);
		~device();

		device(const device&) = delete;
		device& operator=(const device&) = delete;

		device(device&&) noexcept;
		device& operator=(device&&) noexcept;


		const VkDevice get() const;
		const VkQueue getGraphicQueue() const;
		const VkQueue getPresentQueue() const;
};