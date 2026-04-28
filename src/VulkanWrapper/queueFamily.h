#pragma once

#include <vulkan/vulkan.h>
#include <optional>
#include <vector>

using namespace std;

/**
 * @brief 
 * Le struct QueueFamiliesIndices garde les indices des queues
 */
struct QueueFamiliesIndices {
	optional<uint32_t> graphicFamily;
	optional<uint32_t> presentFamily;
	optional<uint32_t> computeFamily;

	bool isComplete();
};

QueueFamiliesIndices findQueueFamilies(VkPhysicalDevice, VkSurfaceKHR);