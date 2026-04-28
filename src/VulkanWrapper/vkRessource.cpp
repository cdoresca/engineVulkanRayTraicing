#include"vkRessource.h"
#include <Shape/mesh.h>

vkRessource::vkRessource(GLFWwindow* w)
{
	instance_ptr = make_unique<instance>();
	surface_ptr = make_unique<surface>(instance_ptr.get(), w);
	Feature ft{};
	ft.connect();
	pdevice_ptr = make_unique<physicalDevice>(instance_ptr.get(), 1, surface_ptr.get(), deviceExtensions, ft.features2, ft.featureChain);
	device_ptr = make_unique<device>(pdevice_ptr.get(), surface_ptr.get());
	setDescriptor = make_unique<descriptor>(device_ptr.get());
	linkerFonction();
	createCommandPool();

}

vkRessource::vkRessource(device* d, physicalDevice* p, surface* s):device_ptr(d),pdevice_ptr(p),surface_ptr(s)
{
	createCommandPool();
}
vkRessource::~vkRessource() { cleanup(); }

void vkRessource::createCommandPool() 
{
	QueueFamiliesIndices queueFamilyIndices = findQueueFamilies(pdevice_ptr->get(), surface_ptr->get());

	VkCommandPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	poolInfo.queueFamilyIndex = queueFamilyIndices.graphicFamily.value();

	VK_CHECK(vkCreateCommandPool(device_ptr->get(), &poolInfo, nullptr, &pool));
}

void vkRessource::linkerFonction()
{
	rt.vkCreateAccelerationStructureKHR =
		(PFN_vkCreateAccelerationStructureKHR)vkGetDeviceProcAddr(device_ptr->get(), "vkCreateAccelerationStructureKHR");

	rt.vkDestroyAccelerationStructureKHR =
		(PFN_vkDestroyAccelerationStructureKHR)vkGetDeviceProcAddr(device_ptr->get(), "vkDestroyAccelerationStructureKHR");

	rt.vkGetAccelerationStructureBuildSizesKHR =
		(PFN_vkGetAccelerationStructureBuildSizesKHR)vkGetDeviceProcAddr(device_ptr->get(), "vkGetAccelerationStructureBuildSizesKHR");

	rt.vkGetAccelerationStructureDeviceAddressKHR =
		(PFN_vkGetAccelerationStructureDeviceAddressKHR)vkGetDeviceProcAddr(device_ptr->get(), "vkGetAccelerationStructureDeviceAddressKHR");

	rt.vkCmdBuildAccelerationStructuresKHR =
		(PFN_vkCmdBuildAccelerationStructuresKHR)vkGetDeviceProcAddr(device_ptr->get(), "vkCmdBuildAccelerationStructuresKHR");

	rt.vkCreateRayTracingPipelinesKHR =
		(PFN_vkCreateRayTracingPipelinesKHR)vkGetDeviceProcAddr(device_ptr->get(), "vkCreateRayTracingPipelinesKHR");

	rt.vkGetRayTracingShaderGroupHandlesKHR =
		(PFN_vkGetRayTracingShaderGroupHandlesKHR)vkGetDeviceProcAddr(device_ptr->get(), "vkGetRayTracingShaderGroupHandlesKHR");

	rt.vkCmdTraceRaysKHR =
		(PFN_vkCmdTraceRaysKHR)vkGetDeviceProcAddr(device_ptr->get(), "vkCmdTraceRaysKHR");
}

VkCommandBuffer vkRessource::createTmpCmdBuffer(VkCommandBufferUsageFlags flags, VkCommandBufferInheritanceInfo* info) const{
	VkCommandBuffer tmp;

	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = pool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = 1;
	
	VK_CHECK(vkAllocateCommandBuffers(device_ptr->get(), &allocInfo, &tmp));

	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = flags;
	beginInfo.pInheritanceInfo = info;

	VK_CHECK(vkBeginCommandBuffer(tmp, &beginInfo));
	
	return tmp;
}

unique_ptr<buffer> vkRessource::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties) const {
	unique_ptr<buffer> tmp = make_unique<buffer>(device_ptr.get());

	createBuffer(size, usage,  properties, tmp->get(), tmp->getMemory());
	return tmp;
}

unique_ptr<buffer> vkRessource::createVertexBuffer(const vector<glm::vec3> vertices, bool ray)
{
	VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();
	VkBufferUsageFlags usage = ray ? VK_BUFFER_USAGE_TRANSFER_DST_BIT| 
		VK_BUFFER_USAGE_VERTEX_BUFFER_BIT |
		VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT |
		VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR |
		VK_BUFFER_USAGE_STORAGE_BUFFER_BIT :
		VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
	
	unique_ptr<buffer> tmp = createBuffer(bufferSize, usage, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	staggingBuffer(bufferSize, usage, tmp->get(), (void*)vertices.data());
	return tmp;
}
unique_ptr<buffer> vkRessource::createVertexBuffer(const vector<vertex> vertices, bool ray)
{
	VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();
	VkBufferUsageFlags usage = ray ? VK_BUFFER_USAGE_TRANSFER_DST_BIT |
		VK_BUFFER_USAGE_VERTEX_BUFFER_BIT |
		VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT |
		VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR |
		VK_BUFFER_USAGE_STORAGE_BUFFER_BIT:
		VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;

	unique_ptr<buffer> tmp = createBuffer(bufferSize, usage, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	staggingBuffer(bufferSize, usage, tmp->get(), (void*)vertices.data());
	return tmp;
}

unique_ptr<buffer> vkRessource::createIndexBuffer(const vector<uint32_t> index, bool ray)
{
	VkDeviceSize bufferSize = sizeof(index[0]) * index.size();
	VkBufferUsageFlags usage = ray ? VK_BUFFER_USAGE_INDEX_BUFFER_BIT |
		VK_BUFFER_USAGE_TRANSFER_DST_BIT|
		VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT |
		VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR |
		VK_BUFFER_USAGE_STORAGE_BUFFER_BIT :
		VK_BUFFER_USAGE_TRANSFER_DST_BIT|
		VK_BUFFER_USAGE_INDEX_BUFFER_BIT;

	unique_ptr<buffer> tmp = createBuffer(bufferSize, usage, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	staggingBuffer(bufferSize, usage, tmp->get(), (void*)index.data());
	return tmp;
}


void vkRessource::cleanup() {
	
	vkDestroyCommandPool(device_ptr->get(), pool, nullptr);
}

const VkDevice vkRessource::getDevice() const {
	return device_ptr->get();
}

const VkQueue vkRessource::getGraphicQueue() const {
	return device_ptr->getGraphicQueue();
}

const VkCommandPool vkRessource::getCommandPool() const {
	return pool;
}

const VkPhysicalDevice vkRessource::getPhysicalDevice() const {
	return pdevice_ptr->get();
}

const VkSurfaceKHR vkRessource::getSurface() const {
	return surface_ptr->get();
}

const VkInstance vkRessource::getInstance() const
{
	return instance_ptr->get();
}

VkDescriptorSet vkRessource::getSet(uint32_t index, uint32_t offset) const
{
	return setDescriptor->descriptorSet[setDescriptor->getCount() * index + offset];
}

uint32_t vkRessource::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) const {
	VkPhysicalDeviceMemoryProperties memProperties;
	vkGetPhysicalDeviceMemoryProperties(pdevice_ptr->get(), &memProperties);

	for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
		if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
			return i;
		}
	}

	throw std::runtime_error("failed to find suitable memory type!");
}

void vkRessource::flushCommandBuffer(VkCommandBuffer commandBuffer, bool free)
{
	if (commandBuffer == VK_NULL_HANDLE)
	{
		return;
	}

	VK_CHECK(vkEndCommandBuffer(commandBuffer));

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;
	submitInfo.waitSemaphoreCount = 0;
	submitInfo.pWaitSemaphores = nullptr;
	submitInfo.signalSemaphoreCount = 0;
	submitInfo.pSignalSemaphores = nullptr;



	VkFenceCreateInfo fenceInfo{};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	VkFence fence;

	VK_CHECK(vkCreateFence(device_ptr->get(), &fenceInfo, nullptr, &fence));
	VK_CHECK(vkQueueSubmit(device_ptr->getGraphicQueue(), 1, &submitInfo, fence));
	VK_CHECK(vkWaitForFences(device_ptr->get(), 1, &fence, VK_TRUE, DEFAULT_FENCE_TIMEOUT));
	vkDestroyFence(device_ptr->get(), fence, nullptr);
	if (free)
	{
		vkFreeCommandBuffers(device_ptr->get(), pool, 1, &commandBuffer);
	}
}

void vkRessource::setImageLayout(
	VkCommandBuffer cmdbuffer,
	VkImage image,
	VkImageLayout oldImageLayout,
	VkImageLayout newImageLayout,
	VkImageSubresourceRange subresourceRange,
	VkPipelineStageFlags srcStageMask,
	VkPipelineStageFlags dstStageMask) {

	VkImageMemoryBarrier barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.oldLayout = oldImageLayout;
	barrier.newLayout = newImageLayout;
	barrier.image = image;
	barrier.subresourceRange = subresourceRange;

	switch (oldImageLayout)
	{
	case VK_IMAGE_LAYOUT_UNDEFINED:
		barrier.srcAccessMask = 0;
		break;

	case VK_IMAGE_LAYOUT_GENERAL:
		barrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT | VK_ACCESS_SHADER_READ_BIT;
		break;

	case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		break;

	case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		break;

	case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
		barrier.srcAccessMask =
			VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
			VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
		break;

	case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
		barrier.srcAccessMask = 0;
		break;

	default:
		break;
	}
	switch (newImageLayout)
	{
	case VK_IMAGE_LAYOUT_GENERAL:
		barrier.dstAccessMask = VK_ACCESS_SHADER_WRITE_BIT | VK_ACCESS_SHADER_READ_BIT;
		break;

	case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		break;

	case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		break;

	case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
		barrier.dstAccessMask =
			VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
			VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
		break;

	case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
		barrier.dstAccessMask = 0;
		break;

	default:
		break;
	}

	vkCmdPipelineBarrier(
		cmdbuffer,
		srcStageMask,
		dstStageMask,
		0,
		0, nullptr,
		0, nullptr,
		1, &barrier);
}

void vkRessource::createDescriptorPool(vector<VkDescriptorPoolSize> poolSizes)
{
	setDescriptor->createDescriptorPool(poolSizes);
}

void vkRessource::addDescriptorSetLayout(vector<VkDescriptorType> type, vector<VkShaderStageFlags> flags, uint32_t sizeDescriptor)
{
	if (sizeDescriptor == 1)
		setDescriptor->addDescriptorSetLayout(type, flags);
	else  setDescriptor->addDescriptorSetLayoutBindless(type, flags, sizeDescriptor);
}

void vkRessource::addPushConstant(VkShaderStageFlags flag,uint32_t size)
{
	setDescriptor->addPushConstant(flag ,size);
}

void vkRessource::allocateDescriptorSet()
{
	setDescriptor->allocateDescriptorSets();
}

const vector<VkDescriptorSetLayout>& vkRessource::getDescriptorSetLayout() const
{
	return setDescriptor->setLayout;
}

const vector<VkPushConstantRange>& vkRessource::getPushConstant() const
{
	return setDescriptor->pushConstant;
}

void vkRessource::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) const{
	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = pool;
	allocInfo.commandBufferCount = 1;

	VkCommandBuffer commandBuffer;
	VK_CHECK(vkAllocateCommandBuffers(device_ptr->get(), &allocInfo, &commandBuffer));

	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	VK_CHECK(vkBeginCommandBuffer(commandBuffer, &beginInfo));

	VkBufferCopy copyRegion{};
	copyRegion.size = size;
	vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

	VK_CHECK(vkEndCommandBuffer(commandBuffer));

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;
	submitInfo.waitSemaphoreCount = 0;
	submitInfo.pWaitSemaphores = nullptr;
	submitInfo.signalSemaphoreCount = 0;
	submitInfo.pSignalSemaphores = nullptr;


	VK_CHECK(vkQueueSubmit(device_ptr->getGraphicQueue(), 1, &submitInfo, VK_NULL_HANDLE));
	VK_CHECK(vkQueueWaitIdle(device_ptr->getGraphicQueue()));

	vkFreeCommandBuffers(device_ptr->get(), pool, 1, &commandBuffer);
}


VkDeviceAddress vkRessource::queryBufferAddress(VkBuffer other) const{
	VkBufferDeviceAddressInfo info{};
	info.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
	info.buffer = other;

	return vkGetBufferDeviceAddress(device_ptr->get(), &info);
}

void vkRessource::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer_, VkDeviceMemory& bufferMemory) const {

	if (size == 0) {
		int i = 0;
	}
	VkBufferCreateInfo bufferInfo{};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = size;
	bufferInfo.usage = usage;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	VK_CHECK(vkCreateBuffer(device_ptr->get(), &bufferInfo, nullptr, &buffer_));
		
	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(device_ptr->get(), buffer_, &memRequirements);

	VkMemoryAllocateFlagsInfo allocFlags{};
	allocFlags.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO;
	allocFlags.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;

	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);
	allocInfo.pNext = &allocFlags;

	VK_CHECK(vkAllocateMemory(device_ptr->get(), &allocInfo, nullptr, &bufferMemory));
	VK_CHECK(vkBindBufferMemory(device_ptr->get(), buffer_, bufferMemory, 0));
}


VkDeviceAddress vkRessource::queryASAddress(VkAccelerationStructureKHR as) const {
	VkAccelerationStructureDeviceAddressInfoKHR info{};
	info.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR;
	info.accelerationStructure = as;

	return rt.vkGetAccelerationStructureDeviceAddressKHR(device_ptr->get(), &info);

}

void vkRessource::staggingBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkBuffer b, void* data) {

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingMemory;

	createBuffer(
		size,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT|usage,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		stagingBuffer,
		stagingMemory
	);

	void* mapped;
	VK_CHECK(vkMapMemory(device_ptr->get(), stagingMemory, 0, size, 0, &mapped));
	memcpy(mapped, data, size);

	copyBuffer(stagingBuffer, b, size);

	vkDestroyBuffer(device_ptr->get(), stagingBuffer, nullptr);
	vkFreeMemory(device_ptr->get(), stagingMemory, nullptr);
}

const VkQueue vkRessource::getPresentQueue() const {
	return device_ptr->getPresentQueue();
}




void Feature::connect() {
	featureChain.push_back(reinterpret_cast<VkBaseOutStructure*>(&accelFeatures));
	featureChain.push_back(reinterpret_cast<VkBaseOutStructure*>(&rtFeatures));
	featureChain.push_back(reinterpret_cast<VkBaseOutStructure*>(&bufferAddressFeatures));
	featureChain.push_back(reinterpret_cast<VkBaseOutStructure*>(&indexing));

	for (size_t i = 0; i < featureChain.size() - 1; i++) {
		auto* base = reinterpret_cast<VkBaseOutStructure*>(featureChain[i]);
		base->pNext = featureChain[i + 1];
	}

	reinterpret_cast<VkBaseOutStructure*>(featureChain.back())->pNext = nullptr;

	features2.pNext = featureChain[0];

}