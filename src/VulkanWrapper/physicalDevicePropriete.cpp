#include"physicalDevicePropriete.h"

physicalDevicePropriete queryphysicalDevicePropriete(VkPhysicalDevice dev) {
	physicalDevicePropriete prop;
	prop.m_asProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_PROPERTIES_KHR;

	prop.m_rtProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_PROPERTIES_KHR;
	prop.m_rtProperties.pNext = &prop.m_asProperties;

	prop.indexingProps.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_PROPERTIES;
	prop.indexingProps.pNext = &prop.m_rtProperties;

	prop.prop2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
	prop.prop2.pNext = &prop.indexingProps;

	vkGetPhysicalDeviceProperties2(dev, &prop.prop2);

	return prop;
}