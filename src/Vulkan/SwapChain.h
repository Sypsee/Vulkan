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
        SwapChain(Device &deviceRef, VkExtent2D deviceExtent);
        ~SwapChain();

    private:
        void init();
        void createSwapChain();
        void createImageViews();
        VkSurfaceFormatKHR chooseSwapSurfaceFormat(std::vector<VkSurfaceFormatKHR>& availableFormats);
        VkPresentModeKHR choosePresentMode(std::vector<VkPresentModeKHR> &availablePresentModes);

        VkSwapchainKHR m_VkSwapChain;
        std::vector<VkImage> m_SwapChainImages;
        std::vector<VkImageView> m_SwapChainImageViews;
        SwapChainSupportDetails m_SwapChainSupport;
        VkFormat m_SwapChainImageFormat;
        Device &m_Device;
        VkExtent2D m_DeviceExtent;
    };
}
