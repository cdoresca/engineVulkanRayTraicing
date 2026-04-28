#include "physicalDevice.h"

physicalDevice::physicalDevice(instance* inst,const int type, surface* surf, const vector<const char*>& ext, VkPhysicalDeviceFeatures2& ft, const vector<VkBaseOutStructure*>& ft_vt):deviceExtensions(ext), type(type), deviceFeature(ft_vt), feature2(ft),surface_ptr(surf),instance_ptr(inst) {
    pickPhysicalDevice();

    if (type == 0) {
        f = reinterpret_cast<VkBaseOutStructure*>(&feature);
    }
    else {
        f = reinterpret_cast<VkBaseOutStructure*>(&feature2);
        
    }
}

physicalDevice::~physicalDevice() { cleanup(); }

bool physicalDevice::isDeviceSuitable(const VkPhysicalDevice gpu)
{
	return type == 0 ? isQueueSuitable(gpu) && isExtensionSuitable(gpu) && isSwapChainSuitable(gpu) : isQueueSuitable(gpu) && isExtensionSuitable(gpu) && isSwapChainSuitable(gpu) && isFeatureSuitable(gpu);
}
bool physicalDevice::isQueueSuitable(const VkPhysicalDevice gpu) const{
	QueueFamiliesIndices indices = findQueueFamilies(gpu,surface_ptr->get());

	return indices.isComplete();
}

bool physicalDevice::isSwapChainSuitable(const VkPhysicalDevice gpu) const{
	SwapChainSupportDetails swapChainSupport = querySwapChainSupport(gpu, surface_ptr->get());
	return !swapChainSupport.formats.empty() && !swapChainSupport.presentsModes.empty();
}

bool physicalDevice::isExtensionSuitable(const VkPhysicalDevice gpu) const{
	uint32_t extensionCount;
    VK_CHECK(vkEnumerateDeviceExtensionProperties(gpu, nullptr, &extensionCount, nullptr));

	vector<VkExtensionProperties> availableExtensions(extensionCount);
    VK_CHECK(vkEnumerateDeviceExtensionProperties(gpu, nullptr, &extensionCount, availableExtensions.data()));

	set<string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());
	for (const auto& extension : availableExtensions)
		requiredExtensions.erase(extension.extensionName);

	return requiredExtensions.empty();
}

bool physicalDevice::isFeatureSuitable(const VkPhysicalDevice gpu)
{
	vkGetPhysicalDeviceFeatures2(gpu, &feature2);

	for (void* ft: deviceFeature) {
        auto* base = reinterpret_cast<VkBaseOutStructure*>(ft);

        switch (base->sType)
        {
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR:
            {
                auto* asf = reinterpret_cast<VkPhysicalDeviceAccelerationStructureFeaturesKHR*>(ft);
                if (!asf->accelerationStructure)
                    return false;
                break;
            }

            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR:
            {
                auto* rtf = reinterpret_cast<VkPhysicalDeviceRayTracingPipelineFeaturesKHR*>(ft);
                if (!rtf->rayTracingPipeline)
                    return false;
                break;
            }

            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES:
            {
                auto* bda = reinterpret_cast<VkPhysicalDeviceBufferDeviceAddressFeatures*>(ft);
                if (!bda->bufferDeviceAddress)
                    return false;
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES: 
            {
                auto* index = reinterpret_cast<VkPhysicalDeviceDescriptorIndexingFeatures*>(ft);
                if (!index->descriptorBindingPartiallyBound || 
                    !index->descriptorBindingVariableDescriptorCount ||
                    !index->runtimeDescriptorArray)
                    return false;
                break;
            }
        }
	}
    return true;
}

void physicalDevice::pickPhysicalDevice(){
    uint32_t deviceCount = 0;

    VK_CHECK(vkEnumeratePhysicalDevices(instance_ptr->get(), &deviceCount, nullptr));

    vector<VkPhysicalDevice> devices(deviceCount);
    VK_CHECK(vkEnumeratePhysicalDevices(instance_ptr->get(), &deviceCount, devices.data()));

    for (const auto& gpu : devices) {
        if (isDeviceSuitable(gpu)) {
            device = gpu;
            break;
        }
    }

    if (device == VK_NULL_HANDLE)
        throw runtime_error("failed to find a suitable GPU");
}


physicalDevice::physicalDevice(physicalDevice&& other) noexcept :type(other.type) {
    moveFrom(other);
}
physicalDevice& physicalDevice::operator=(physicalDevice&& other) noexcept {
    if (this != &other) {
        cleanup();
        moveFrom(other);
    }
    return *this;
}



void physicalDevice::moveFrom(physicalDevice& other) {

    device = other.device;
    feature = other.feature;
    surface_ptr = other.surface_ptr;
    instance_ptr = other.instance_ptr;
    deviceExtensions = other.deviceExtensions;
    deviceFeature = other.deviceFeature;

    other.cleanup();
}

const VkPhysicalDevice physicalDevice::get() const { return device; }

const VkBaseOutStructure* physicalDevice::getFeature() const { return f; }
const vector<const char*>& physicalDevice::getExtension() const { return deviceExtensions; }

void physicalDevice::cleanup() {
    device = VK_NULL_HANDLE;
    surface_ptr = nullptr;
    instance_ptr = nullptr;
}