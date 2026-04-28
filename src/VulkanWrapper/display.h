#pragma once

#include "swapChainSupportDetail.h"
#include "queueFamily.h"
#include "vkRessource.h"

/**
 * @brief 
 * Le struct StorageImage les informations nécessaires pour utiliser des 
 * VkImages offscreen.
 */
struct StorageImage {
	VkDeviceMemory memory;
	VkImage image;
	VkImageView view;
	VkFormat format;
};

/**
 * @brief 
 * Type d'images offscreen.
 */
enum Image {
	CURRENT,
	HISTORY,
	OUTPUT
};

/**
 * @brief 
 * La class display gère le VkSwapChaiinKHR, VkRenderPass et le VkFrameBuffer
 */
class display {

	GLFWwindow* window;

	VkSwapchainKHR swapChain;
	VkFormat swapChainImageFormat;
	VkExtent2D swapChainExtent;
	VkRenderPass renderPass;

	vkRessource* vk;

	vector<VkImage> swapChainImages;
	vector<VkImageLayout> swapChainImagesLayout;
	vector<VkImageView> swapChainImagesViews;
	vector<VkFramebuffer> swapChainFramebuffers;

	array<StorageImage,MAX_FRAMES_IN_FLIGHT> currentFrame;
	array<StorageImage,MAX_FRAMES_IN_FLIGHT> historyFrame;
	array<StorageImage,MAX_FRAMES_IN_FLIGHT> outputFrame;

	uint32_t imageCount;
	uint32_t minImageCount;

	void createSwapChain();
	void createImageViews();
	void createFramebuffers();
	void createRenderPass();
	void createAttachementDescription(VkAttachmentDescription&);
	void createAttachementReference(VkAttachmentReference&);
	void createSubpassDescription(VkSubpassDescription&, VkAttachmentReference&);
	void createSubpassDependency(VkSubpassDependency&);
	void createStorageImage(StorageImage& st);
	void updateDescriptorStorageImage(VkDescriptorSet set, StorageImage& st,  uint32_t index);
	void cleanup();
	void moveFrom(display&);
	void cleanupSwapChain();
	
	static void framebufferResizeCallback(GLFWwindow* window, int width, int height);

	
	public:
		bool framebufferResized = false;

		display(GLFWwindow*, vkRessource*);
		~display();
		
		const VkExtent2D& getExtent() const;
		const VkRenderPass& getRenderPass() const;
		const StorageImage& getStorage(Image name,int) const;
		const VkFramebuffer& getFrameBuffer(int index) const;
		const VkImage& getImage(int index) const;
		const uint32_t getImageCount();
		const uint32_t getMinImageCount();
		VkSwapchainKHR getSwapChain() const;
		VkImageLayout getLayout(uint32_t);


		void recreateSwapChain();
		void setLayout(uint32_t, VkImageLayout);

		display(const display&) = delete;
		display& operator=(const display&) = delete;

		display(display&&) noexcept;
		display& operator=(display&&) noexcept;

};		

void cleanupStorageImage(VkDevice dev, StorageImage img);