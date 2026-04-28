#pragma once
#include "vulkan/vulkan.h"

/**
 * @brief 
 * La struct physicalDevicePropriete contient les propriétés du processeur graphique.
 * 
 */
struct physicalDevicePropriete {
	VkPhysicalDeviceRayTracingPipelinePropertiesKHR m_rtProperties{};
	VkPhysicalDeviceAccelerationStructurePropertiesKHR m_asProperties{};
	VkPhysicalDeviceDescriptorIndexingProperties indexingProps{};
	VkPhysicalDeviceProperties2 prop2{};
};

physicalDevicePropriete queryphysicalDevicePropriete(VkPhysicalDevice);