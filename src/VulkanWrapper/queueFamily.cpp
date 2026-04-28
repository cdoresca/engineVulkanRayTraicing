#include "queueFamily.h"

bool QueueFamiliesIndices::isComplete() {	return graphicFamily.has_value() && presentFamily.has_value() && computeFamily.has_value();	}

QueueFamiliesIndices findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface) {
	QueueFamiliesIndices indices;

	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

	vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

	int i = 0;
	for (const auto& queueFamily : queueFamilies) {
		VkBool32 presentSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);

		if (presentSupport)
			indices.presentFamily = i;
		if (queueFamily.queueFlags && VK_QUEUE_GRAPHICS_BIT)
			indices.graphicFamily = i;
		if (queueFamily.queueFlags && VK_QUEUE_COMPUTE_BIT)
			indices.computeFamily = i;
		if (indices.isComplete())
			break;
		i++;
	}
	return indices;
}
