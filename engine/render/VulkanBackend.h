#pragma once
#include "RHI.h"
#include <vulkan/vulkan.h>
#include <vector>
#include <SDL2/SDL.h>

#include "../ecs/World.h"
#include "../ecs/Components.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace mmo::render {

    struct PushConstantData {
        glm::mat4 mvp;
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

    private:
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

        bool CreateInstance();
        bool CreateSurface();
        bool PickPhysicalDevice();
        bool CreateLogicalDevice();
        bool CreateSwapchain();
        bool CreateImageViews();
        bool CreateRenderPass();
        bool CreateGraphicsPipeline();
        bool CreateDepthResources();
        VkShaderModule CreateShaderModule(const std::vector<char>& code);
        bool CreateFramebuffers();
        bool CreateCommandPool();
        bool CreateCommandBuffer();
        bool CreateSyncObjects();
        uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
    };
}
