#include "mesh.h"

VkVertexInputBindingDescription vertex::getBindingDescription() {
    VkVertexInputBindingDescription bindingDescription{};
    bindingDescription.binding = 0;
    bindingDescription.stride = sizeof(vertex);
    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    return bindingDescription;
}

array<VkVertexInputAttributeDescription, 2> vertex::getAttributeDescriptions() {
    std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions{};
    attributeDescriptions[0].binding = 0;
    attributeDescriptions[0].location = 0;
    attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[0].offset = offsetof(vertex, pos);
    attributeDescriptions[1].binding = 0;
    attributeDescriptions[1].location = 1;
    attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[1].offset = offsetof(vertex, normal);
    return attributeDescriptions;
}
glm::mat4 getMatrixParent(node* them)
{
    vector<glm::mat4> result;
    
    node* parent = them->parent;
    node* tmp;
    while(parent != nullptr){
        tmp = parent;
        parent = parent->parent;
        result.push_back(tmp->model);
    }

    result.pop_back();

   
        glm::mat4 model = glm::mat4(1.0f);
        for (int i = result.size() - 1; i >= 0; i--) {
            model = result[i] * model;
        }
   
        return model;

}