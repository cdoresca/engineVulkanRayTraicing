#include "device.h"

device::device(physicalDevice* pd,surface* surf):physicalDevice_ptr(pd),surface_ptr(surf) {
	createLogicalDevice();
	CreateQueue();
}

device::~device() { cleanup(); }

void device::createInfoQueue(vector<VkDeviceQueueCreateInfo>& info) const{
	QueueFamiliesIndices indices = findQueueFamilies(physicalDevice_ptr->get(),surface_ptr->get());

	set<uint32_t> uniqueQueueFamilies = { indices.graphicFamily.value(), indices.presentFamily.value() };

	
	for (uint32_t queueFamily : uniqueQueueFamilies) {
		VkDeviceQueueCreateInfo	queueCreateInfo{};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = queueFamily;
		queueCreateInfo.queueCount = 1;
		queueCreateInfo.pQueuePriorities = &priorityQueue;
		info.push_back(queueCreateInfo);
	}
}

void device::createLogicalDevice() {

	vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	createInfoQueue(queueCreateInfos);

	VkDeviceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
	createInfo.pQueueCreateInfos = queueCreateInfos.data();
	createInfo.pNext = physicalDevice_ptr->getFeature();
	createInfo.enabledExtensionCount = static_cast<uint32_t>(physicalDevice_ptr->getExtension().size());
	createInfo.ppEnabledExtensionNames = physicalDevice_ptr->getExtension().data();
	if (enableValidationLayers)
	{
		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		createInfo.ppEnabledLayerNames = validationLayers.data();
	}
	else {
		createInfo.enabledLayerCount = 0;
	}
	VK_CHECK(vkCreateDevice(physicalDevice_ptr->get(), &createInfo, nullptr, &m_device));
}

void device::CreateQueue() {
	QueueFamiliesIndices indices = findQueueFamilies(physicalDevice_ptr->get(), surface_ptr->get());

	vkGetDeviceQueue(m_device, indices.graphicFamily.value(), 0, &graphicsQueue);
	vkGetDeviceQueue(m_device, indices.presentFamily.value(), 0, &presentQueue);
	vkGetDeviceQueue(m_device, indices.computeFamily.value(), 0, &computeQueue);
}

void device::cleanup() {

	vkDestroyDevice(m_device, nullptr);
	presentQueue = VK_NULL_HANDLE;
	graphicsQueue = VK_NULL_HANDLE;
}

device::device(device&& other) noexcept {
	moveFrom(other);
}

device& device::operator=(device&& other) noexcept {
	if (this != &other) {
		cleanup();
		moveFrom(other);
	}
	return *this;
}


void device::moveFrom(device& other) {
	physicalDevice_ptr = other.physicalDevice_ptr;
	surface_ptr = other.surface_ptr;
	presentQueue = other.presentQueue;
	graphicsQueue = other.graphicsQueue;
	other.m_device = VK_NULL_HANDLE;
	other.cleanup();
}



const VkDevice device::get() const { return m_device; }

const VkQueue device::getGraphicQueue() const { return graphicsQueue; }

const VkQueue device::getPresentQueue() const { return presentQueue; }