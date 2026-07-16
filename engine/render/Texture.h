#pragma once
#include <vulkan/vulkan.h>
#include <string>

namespace mmo::render {

    struct Texture {
        VkImage image = VK_NULL_HANDLE;
        void* allocation = nullptr; // VmaAllocation
        VkImageView imageView = VK_NULL_HANDLE;
        VkSampler sampler = VK_NULL_HANDLE;
        
        // The descriptor set that binds this texture to the shader
        VkDescriptorSet descriptorSet = VK_NULL_HANDLE;
    };

}
