#pragma once
#include <glm/glm.hpp>
#include <vector>
#include <vulkan/vulkan.h>

namespace mmo::render {

    struct Vertex {
        glm::vec3 pos;
        glm::vec3 normal;
        glm::vec3 color;
        glm::vec2 texCoord;
    };

    struct Mesh {
        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;
        
        // Vulkan resources for the mesh
        VkBuffer vertexBuffer = VK_NULL_HANDLE;
        VkBuffer indexBuffer = VK_NULL_HANDLE;
        
        // Use void* here to avoid including vk_mem_alloc.h everywhere
        void* vertexAllocation = nullptr; 
        void* indexAllocation = nullptr;
    };

}
