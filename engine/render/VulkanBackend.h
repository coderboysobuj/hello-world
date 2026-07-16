#pragma once
#include "RHI.h"
#include <vulkan/vulkan.h>
#include <vector>
#include <SDL2/SDL.h>

#include "../ecs/World.h"
#include "../ecs/Components.h"
#include "Mesh.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace mmo::render {

    struct PushConstantData {
        glm::mat4 mvp;
        glm::vec4 colorMultiplier;
    };

    class VulkanBackend : public RHI {
    public:
        VulkanBackend();
        ~VulkanBackend() override;

        bool Initialize(void* windowHandle) override;
        void Shutdown() override;
        void BeginFrame() override;
        void EndFrame() override;
        void RenderEntities(mmo::ecs::World& ecsWorld);
        void SetCamera(const glm::vec3& position, const glm::vec3& target);

        bool CreateMeshBuffers(Mesh& mesh);
        void DestroyMesh(Mesh& mesh);

    private:
        void* m_allocator = nullptr; // Actually VmaAllocator
        glm::vec3 m_cameraPos{0.0f, 5.0f, 12.0f};
        glm::vec3 m_cameraTarget{0.0f, -1.0f, 0.0f};
        SDL_Window* m_window = nullptr;

        VkInstance m_instance = VK_NULL_HANDLE;
        VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
        VkDevice m_device = VK_NULL_HANDLE;
        uint32_t m_graphicsQueueFamily = -1;
        uint32_t m_presentQueueFamily = -1;
        VkQueue m_graphicsQueue = VK_NULL_HANDLE;
        VkQueue m_presentQueue = VK_NULL_HANDLE;
        VkSurfaceKHR m_surface = VK_NULL_HANDLE;

        VkSwapchainKHR m_swapchain = VK_NULL_HANDLE;
        VkFormat m_swapchainImageFormat;
        VkExtent2D m_swapchainExtent;
        std::vector<VkImage> m_swapchainImages;
        std::vector<VkImageView> m_swapchainImageViews;
        std::vector<VkFramebuffer> m_swapchainFramebuffers;

        VkImage m_depthImage = VK_NULL_HANDLE;
        VkDeviceMemory m_depthImageMemory = VK_NULL_HANDLE;
        VkImageView m_depthImageView = VK_NULL_HANDLE;

        VkRenderPass m_renderPass = VK_NULL_HANDLE;
        VkPipelineLayout m_pipelineLayout = VK_NULL_HANDLE;
        VkPipeline m_graphicsPipeline = VK_NULL_HANDLE;
        
        VkCommandPool m_commandPool = VK_NULL_HANDLE;
        VkCommandBuffer m_commandBuffer = VK_NULL_HANDLE;

        VkSemaphore m_imageAvailableSemaphore = VK_NULL_HANDLE;
        VkSemaphore m_renderFinishedSemaphore = VK_NULL_HANDLE;
        VkFence m_inFlightFence = VK_NULL_HANDLE;

        uint32_t m_imageIndex = 0;

        VkDescriptorSetLayout m_descriptorSetLayout = VK_NULL_HANDLE;
        VkDescriptorPool m_descriptorPool = VK_NULL_HANDLE;

        bool CreateInstance();
        bool CreateSurface();
        bool PickPhysicalDevice();
        bool CreateLogicalDevice();
        bool CreateSwapchain();
        bool CreateImageViews();
        bool CreateRenderPass();
        bool CreateDescriptorSetLayout();
        bool CreateGraphicsPipeline();
        bool CreateDepthResources();
        VkShaderModule CreateShaderModule(const std::vector<char>& code);
        bool CreateFramebuffers();
        bool CreateCommandPool();
        bool CreateDescriptorPool();
        bool CreateCommandBuffer();
        bool CreateSyncObjects();
        uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

    public:
        // Texture handling
        bool CreateTexture(const std::string& filepath, class Texture& outTexture);
        void DestroyTexture(class Texture& texture);
        
    private:
        VkCommandBuffer BeginSingleTimeCommands();
        void EndSingleTimeCommands(VkCommandBuffer commandBuffer);
        void TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
        void CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
    };
}
