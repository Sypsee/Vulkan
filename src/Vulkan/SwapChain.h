#pragma once

#include "Device.h"

// vulkan headers
#include <vulkan/vulkan.h>

// std lib headers
#include <memory>
#include <string>
#include <vector>

namespace vk
{
    class SwapChain
    {
    public:
        static constexpr int MAX_FRAMES_IN_FLIGHT = 2;

        SwapChain(Device &deviceRef, VkExtent2D deviceExtent);
        SwapChain(Device &deviceRef, VkExtent2D deviceExtent, std::shared_ptr<SwapChain> prev);
        ~SwapChain();

        SwapChain(const SwapChain &) = delete;
        SwapChain &operator=(const SwapChain &) = delete;

        VkResult acquireNextImage(uint32_t *imageIndex);
        VkResult submitCommandBuffers(const VkCommandBuffer *buffers, uint32_t *imageIndex);

        VkSwapchainKHR getSwapChain() const {return m_VkSwapChain;}
        VkExtent2D getDeviceExtent() const {return m_DeviceExtent;}
        VkRenderPass getRenderPass() const {return m_VkRenderPass;}
        VkFramebuffer getFramebuffer(const uint32_t i) const {return m_SwapChainFramebuffers[i];}
        uint32_t getImageCount() { return m_SwapChainImageViews.size(); }

    private:
        void init();
        void createSwapChain();
        void createImageViews();
        void createRenderPass();
        void createFramebuffers();
        void createSyncObjects();
        VkSurfaceFormatKHR chooseSwapSurfaceFormat(std::vector<VkSurfaceFormatKHR>& availableFormats);
        VkPresentModeKHR choosePresentMode(std::vector<VkPresentModeKHR> &availablePresentModes);

        VkSwapchainKHR m_VkSwapChain;
        VkRenderPass m_VkRenderPass;
        std::vector<VkImage> m_SwapChainImages;
        std::vector<VkImageView> m_SwapChainImageViews;
        std::vector<VkFramebuffer> m_SwapChainFramebuffers;
        std::vector<VkSemaphore> m_ImageAvailableSemaphores;
        std::vector<VkSemaphore> m_RenderFinishedSemaphores;
        std::vector<VkFence> m_InFlightFences;
        std::vector<VkFence> m_ImagesInFlight;
        SwapChainSupportDetails m_SwapChainSupport;
        VkFormat m_SwapChainImageFormat;
        Device &m_Device;
        VkExtent2D m_DeviceExtent;

        std::shared_ptr<SwapChain> m_OldSwapChain = nullptr;

        size_t m_CurrentFrame = 0;
    };
}
