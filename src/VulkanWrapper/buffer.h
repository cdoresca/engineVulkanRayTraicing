#pragma once
#include<vulkan/vulkan.h>


#include "device.h"

/**
 * @brief 
 * La class buffer gère la durée de vie de VkBuffer et le VkDeviceMemory 
 * 
 */
class buffer {

	VkBuffer m_buffer;
	VkDeviceMemory bufferMemory;
	
	void* mapped;

	device* device_ptr;

	void cleanup();

	public:
		buffer(device*);
		buffer(device*, VkBuffer&, VkDeviceMemory&, void*);
		~buffer();

		VkBuffer& get() ;
		VkDeviceMemory& getMemory() ;

		VkResult map(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);

		void unmap();
		void* data();

		buffer(const buffer&) = delete;
		buffer& operator=(const buffer&) = delete;
};



