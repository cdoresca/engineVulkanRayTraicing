#include "display.h"

display::display(GLFWwindow* window, vkRessource* v):window(window), vk(v)
{
	createSwapChain();
	createImageViews();
	createRenderPass();
	createFramebuffers();

	for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		createStorageImage(currentFrame[i]);
		createStorageImage(historyFrame[i]);
		createStorageImage(outputFrame[i]);
		VkDescriptorSet set = vk->getSet(i);
		updateDescriptorStorageImage(set, currentFrame[i],1);
		updateDescriptorStorageImage(set, historyFrame[i],2);
		updateDescriptorStorageImage(set, outputFrame[i],3);
		
	}

	glfwSetWindowUserPointer(window, this);
	glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);

}

display::~display() { cleanup(); }



void display::cleanup(){
	cleanupSwapChain();
	vkDestroyRenderPass(vk->getDevice(), renderPass, nullptr);

	vk = nullptr;
	window = nullptr;
}

void display::moveFrom(display& other)
{
	window = other.window;
	swapChain = other.swapChain;
	swapChainExtent = other.swapChainExtent;
	swapChainFramebuffers = other.swapChainFramebuffers;
	swapChainImageFormat = other.swapChainImageFormat;
	swapChainImages = other.swapChainImages;
	swapChainImagesViews = other.swapChainImagesViews;
	currentFrame = other.currentFrame;

	other.cleanup();
}

void display::cleanupSwapChain()
{

	for (auto framebuffer : swapChainFramebuffers)
		vkDestroyFramebuffer(vk->getDevice(), framebuffer, nullptr);
	for (auto imageView : swapChainImagesViews)
		vkDestroyImageView(vk->getDevice(), imageView, nullptr);
	for (auto& storage : currentFrame)
		cleanupStorageImage(vk->getDevice(), storage);
	for (auto& storage : historyFrame)
		cleanupStorageImage(vk->getDevice(), storage);
	for (auto& storage : outputFrame)
		cleanupStorageImage(vk->getDevice(), storage);

	vkDestroySwapchainKHR(vk->getDevice(), swapChain, nullptr);

}

void display::framebufferResizeCallback(GLFWwindow* window, int width, int height)
{
	auto app = reinterpret_cast<display*>(glfwGetWindowUserPointer(window));
	app->framebufferResized = true;
}


void display::createSwapChain() {
	SwapChainSupportDetails swapChainSupport = querySwapChainSupport(vk->getPhysicalDevice(), vk->getSurface());

	VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);

	VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentsModes);

	VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities, window);

	minImageCount = swapChainSupport.capabilities.minImageCount + 1;

	if (swapChainSupport.capabilities.maxImageCount > 0 &&
		minImageCount > swapChainSupport.capabilities.maxImageCount) {
		minImageCount = swapChainSupport.capabilities.maxImageCount;
	}

	VkSwapchainCreateInfoKHR createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = vk->getSurface();
	createInfo.minImageCount = minImageCount;
	createInfo.imageFormat = surfaceFormat.format;
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	createInfo.imageExtent = extent;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	;

	QueueFamiliesIndices indices = findQueueFamilies(vk->getPhysicalDevice(), vk->getSurface());
	uint32_t queueFamilyIndices[] = { indices.graphicFamily.value(), indices.presentFamily.value() };
	if (indices.graphicFamily != indices.presentFamily) {
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = queueFamilyIndices;
	}
	else {
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		createInfo.queueFamilyIndexCount = 0;
		createInfo.pQueueFamilyIndices = nullptr;
	}

	createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	createInfo.presentMode = presentMode;
	createInfo.clipped = VK_TRUE;
	createInfo.oldSwapchain = VK_NULL_HANDLE;

	VK_CHECK(vkCreateSwapchainKHR(vk->getDevice(), &createInfo, nullptr, &swapChain));

	VK_CHECK(vkGetSwapchainImagesKHR(vk->getDevice(), swapChain, &imageCount, nullptr));
	swapChainImages.resize(imageCount);

	VK_CHECK(vkGetSwapchainImagesKHR(vk->getDevice(), swapChain, &imageCount, swapChainImages.data()));

	swapChainImageFormat = surfaceFormat.format;
	swapChainExtent = extent;

	swapChainImagesLayout.resize(swapChainImages.size());

	for (size_t i = 0; i < swapChainImagesLayout.size(); i++)
	{
		swapChainImagesLayout[i] = VK_IMAGE_LAYOUT_UNDEFINED;
	}
}

void  display::createImageViews() {
	swapChainImagesViews.resize(swapChainImages.size());

	for (size_t i = 0; i < swapChainImages.size(); i++) {

		VkImageViewCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		createInfo.image = swapChainImages[i];
		createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		createInfo.format = swapChainImageFormat;
		createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		createInfo.subresourceRange.levelCount = 1;
		createInfo.subresourceRange.baseArrayLayer = 0;
		createInfo.subresourceRange.layerCount = 1;

		VK_CHECK(vkCreateImageView(vk->getDevice(), &createInfo, nullptr, &swapChainImagesViews[i]));
	}
}

void display::createFramebuffers() {
	swapChainFramebuffers.resize(swapChainImagesViews.size());

	for (size_t i = 0; i < swapChainImagesViews.size(); i++) {
		VkImageView attachments[] = {
			swapChainImagesViews[i]
		};

		VkFramebufferCreateInfo framebufferInfo{};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = renderPass;
		framebufferInfo.attachmentCount = 1;
		framebufferInfo.pAttachments = attachments;
		framebufferInfo.width = swapChainExtent.width;
		framebufferInfo.height = swapChainExtent.height;
		framebufferInfo.layers = 1;

		VK_CHECK(vkCreateFramebuffer(vk->getDevice(), &framebufferInfo, nullptr, &swapChainFramebuffers[i]));
	}
}
void display::createAttachementDescription(VkAttachmentDescription& colorAttachment){
	colorAttachment.format = swapChainImageFormat;
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

}

void display::createAttachementReference(VkAttachmentReference& colorAttachmentRef){
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
}
void display::createSubpassDescription(VkSubpassDescription& subpass, VkAttachmentReference& colorAttachmentRef){
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef;
}
void display::createSubpassDependency(VkSubpassDependency& dependency) {
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT;
	dependency.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
								VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
}
void display::createRenderPass() {

	VkAttachmentDescription colorAttachment{};
	createAttachementDescription(colorAttachment);

	VkAttachmentReference colorAttachmentRef{};
	createAttachementReference(colorAttachmentRef);

	VkSubpassDescription subpass{};
	createSubpassDescription(subpass, colorAttachmentRef);

	VkSubpassDependency dependency{};
	createSubpassDependency(dependency);

	VkRenderPassCreateInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = 1;
	renderPassInfo.pAttachments = &colorAttachment;
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	renderPassInfo.dependencyCount = 1;
	renderPassInfo.pDependencies = &dependency;

	VK_CHECK(vkCreateRenderPass(vk->getDevice(), &renderPassInfo, nullptr, &renderPass));

}

const VkExtent2D& display::getExtent() const { return swapChainExtent; }
const VkRenderPass& display::getRenderPass() const { return renderPass; }

void display::createStorageImage(StorageImage& st) {
	VkImageCreateInfo image{};
	image.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	image.imageType = VK_IMAGE_TYPE_2D;
	image.format = VK_FORMAT_R8G8B8A8_UNORM;
	image.extent.width = swapChainExtent.width;
	image.extent.height = swapChainExtent.height;
	image.extent.depth = 1; 
	image.mipLevels = 1;
	image.arrayLayers = 1;
	image.samples = VK_SAMPLE_COUNT_1_BIT;
	image.tiling = VK_IMAGE_TILING_OPTIMAL;
	image.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_STORAGE_BIT;
	image.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

	vkCreateImage(vk->getDevice(), &image, nullptr, &st.image);

	VkMemoryRequirements memReqs;
	vkGetImageMemoryRequirements(vk->getDevice(), st.image, &memReqs);

	VkMemoryAllocateInfo memoryAllocateInfo{};
	memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memoryAllocateInfo.allocationSize = memReqs.size;
	memoryAllocateInfo.memoryTypeIndex = vk->findMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	vkAllocateMemory(vk->getDevice(), &memoryAllocateInfo, nullptr, &st.memory);
	vkBindImageMemory(vk->getDevice(), st.image, st.memory, 0);

	VkImageViewCreateInfo colorImageView{};
	colorImageView.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	colorImageView.viewType = VK_IMAGE_VIEW_TYPE_2D;
	colorImageView.format = VK_FORMAT_R8G8B8A8_UNORM;
	colorImageView.subresourceRange = {};
	colorImageView.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	colorImageView.subresourceRange.baseMipLevel = 0;
	colorImageView.subresourceRange.levelCount = 1;
	colorImageView.subresourceRange.baseArrayLayer = 0;
	colorImageView.subresourceRange.layerCount = 1;
	colorImageView.image = st.image;
	vkCreateImageView(vk->getDevice(), &colorImageView, nullptr, &st.view);

	VkCommandBuffer cmdBuffer = vk->createTmpCmdBuffer();

	vk->setImageLayout(cmdBuffer, st.image,
		VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_GENERAL,
		{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 });
	vk->flushCommandBuffer(cmdBuffer);
}

void display::updateDescriptorStorageImage(VkDescriptorSet set,StorageImage& st, uint32_t index)
{
	
		VkDescriptorImageInfo storageImageDescriptor{};
		storageImageDescriptor.imageView = st.view;
		storageImageDescriptor.imageLayout = VK_IMAGE_LAYOUT_GENERAL;


		VkWriteDescriptorSet imageWrite{};
		imageWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		imageWrite.pNext = nullptr;
		imageWrite.dstSet = set;
		imageWrite.dstBinding = index;
		imageWrite.descriptorCount = 1;
		imageWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
		imageWrite.pImageInfo = &storageImageDescriptor;

		vkUpdateDescriptorSets(vk->getDevice(), 1, &imageWrite, 0, VK_NULL_HANDLE);
	
}


const StorageImage& display::getStorage(Image name,int i) const {
	switch (name) {

		case CURRENT:
			return currentFrame[i];
		case HISTORY:
			return historyFrame[i];
		case OUTPUT:
			return outputFrame[i];
	}
}

const VkFramebuffer& display::getFrameBuffer(int index) const{ return swapChainFramebuffers[index];}

const VkImage& display::getImage(int index) const {
	return swapChainImages[index];
}

const uint32_t display::getImageCount()
{
	return  imageCount;
}

const uint32_t display::getMinImageCount()
{
	return minImageCount;
}


VkSwapchainKHR display::getSwapChain() const {
	return swapChain;
}

VkImageLayout display::getLayout(uint32_t i)
{
	return swapChainImagesLayout[i];
}

void display::recreateSwapChain()
{
	VK_CHECK(vkDeviceWaitIdle(vk->getDevice()));
	int width = 0, height = 0;
	glfwGetFramebufferSize(window, &width, &height);

	while (width == 0 || height == 0) {
		glfwGetFramebufferSize(window, &width, &height);
		glfwWaitEvents(); 
	}
	cleanupSwapChain();

	createSwapChain();
	createImageViews();
	createFramebuffers();
	
	for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		createStorageImage(currentFrame[i]);
		createStorageImage(historyFrame[i]);
		createStorageImage(outputFrame[i]);
		
		VkDescriptorSet set = vk->getSet(i);
		updateDescriptorStorageImage(set, currentFrame[i], 1);
		updateDescriptorStorageImage(set, historyFrame[i], 2);
		updateDescriptorStorageImage(set, outputFrame[i], 3);
	}
}

void display::setLayout(uint32_t i, VkImageLayout l)
{
	swapChainImagesLayout[i] = l;
}

display::display(display&& other) noexcept
{
	moveFrom(other);
}

display& display::operator=(display&& other) noexcept
{
	if (this != &other) {
		cleanup();
		moveFrom(other);
	}
	return *this;
}


void cleanupStorageImage(VkDevice dev, StorageImage img)
{
	vkDestroyImageView(dev, img.view, nullptr);
	vkDestroyImage(dev, img.image, nullptr);
	vkFreeMemory(dev, img.memory, nullptr);
}
