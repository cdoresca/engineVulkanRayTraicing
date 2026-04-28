#include "buffer.h"


void buffer::cleanup() {
    vkDestroyBuffer(device_ptr->get(), m_buffer, nullptr);
    vkFreeMemory(device_ptr->get(), bufferMemory, nullptr);
    mapped = nullptr;
    device_ptr = nullptr;
}


buffer::buffer(device* v, VkBuffer& b, VkDeviceMemory& bm, void* d):device_ptr(v),m_buffer(b),bufferMemory(bm),mapped(d){


}

buffer::buffer(device* v):device_ptr(v) {
    
}
buffer::~buffer() { cleanup(); }


VkBuffer& buffer::get()   {
    return m_buffer;
}

VkDeviceMemory& buffer::getMemory()  {
    return bufferMemory;
}



VkResult buffer::map(VkDeviceSize size, VkDeviceSize offset)
{
    return vkMapMemory(device_ptr->get(), bufferMemory, offset, size, 0, &mapped);
}

void buffer::unmap(){
    if (mapped)
    {
        vkUnmapMemory(device_ptr->get(), bufferMemory);
        mapped = nullptr;
    }
}

void* buffer::data() {
    return mapped;
}