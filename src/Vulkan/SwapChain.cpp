#include "SwapChain.h"

namespace vk
{
    SwapChain::SwapChain(Device &deviceRef, VkExtent2D deviceExtent)
        :m_Device(deviceRef), m_DeviceExtent(deviceExtent)
    {
        init();
    }

    SwapChain::~SwapChain()
    {
        vkDestroySwapchainKHR(m_Device.getDevice(), m_VkSwapChain, NULL);
        for (VkImageView &imageView : m_SwapChainImageViews)
        {
            vkDestroyImageView(m_Device.getDevice(), imageView, NULL);
        }
    }

    void SwapChain::init()
    {
        createSwapChain();
        createImageViews();
    }

    void SwapChain::createSwapChain()
    {
        m_SwapChainSupport = m_Device.querySwapChainSupport(m_Device.getPhysicalDevice());

        VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(m_SwapChainSupport.formats);
        VkPresentModeKHR presentMode = choosePresentMode(m_SwapChainSupport.presentModes);
        
        m_SwapChainImageFormat = surfaceFormat.format;

        uint32_t imageCount = m_SwapChainSupport.capabilities.minImageCount + 1;
        if (m_SwapChainSupport.capabilities.maxImageCount > 0 && imageCount > m_SwapChainSupport.capabilities.maxImageCount)
        {
            imageCount = m_SwapChainSupport.capabilities.maxImageCount;
        }

        VkSwapchainCreateInfoKHR createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.surface = m_Device.getSurface();
        createInfo.minImageCount = imageCount;
        createInfo.imageFormat = surfaceFormat.format;
        createInfo.imageColorSpace = surfaceFormat.colorSpace;
        createInfo.imageExtent = m_DeviceExtent;
        createInfo.imageArrayLayers = 1;
        createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        QueueFamilyIndices indices = m_Device.findQueueFamilies(m_Device.getPhysicalDevice());
        uint32_t queueFamilyIndex[2] = {indices.graphicsFamily.value(), indices.presentFamily.value()};

        if (indices.graphicsFamily != indices.presentFamily)
        {
            createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            createInfo.queueFamilyIndexCount = 2;
            createInfo.pQueueFamilyIndices = queueFamilyIndex;
        }
        else
        {
            createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
            createInfo.queueFamilyIndexCount = 0;
            createInfo.pQueueFamilyIndices = NULL;
        }
        createInfo.preTransform = m_SwapChainSupport.capabilities.currentTransform;
        createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        createInfo.presentMode = presentMode;
        createInfo.clipped = VK_TRUE;
        createInfo.oldSwapchain = VK_NULL_HANDLE;

        if (vkCreateSwapchainKHR(m_Device.getDevice(), &createInfo, NULL, &m_VkSwapChain) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create swap chain!\n");
        }

        vkGetSwapchainImagesKHR(m_Device.getDevice(), m_VkSwapChain, &imageCount, NULL);
        m_SwapChainImages.resize(imageCount);
        vkGetSwapchainImagesKHR(m_Device.getDevice(), m_VkSwapChain, &imageCount, m_SwapChainImages.data());
    }

    void SwapChain::createImageViews()
    {
        m_SwapChainImageViews.resize(m_SwapChainImages.size());

        for (size_t i = 0; i < m_SwapChainImageViews.size(); i++)
        {
            VkImageViewCreateInfo createInfo{};
            createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            createInfo.image = m_SwapChainImages[i];
            createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            createInfo.format = m_SwapChainImageFormat;
            createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            createInfo.subresourceRange.baseMipLevel = 0;
            createInfo.subresourceRange.levelCount = 1;
            createInfo.subresourceRange.baseArrayLayer = 0;
            createInfo.subresourceRange.layerCount = 1;

            if (vkCreateImageView(m_Device.getDevice(), &createInfo, NULL, &m_SwapChainImageViews[i]) != VK_SUCCESS)
            {
                throw std::runtime_error("Failed to create image view!\n");
            }
        }
    }

    VkSurfaceFormatKHR SwapChain::chooseSwapSurfaceFormat(std::vector<VkSurfaceFormatKHR> &availableFormats)
    {
        for (const VkSurfaceFormatKHR &format : availableFormats)
        {
            if (format.format == VK_FORMAT_B8G8R8A8_SRGB && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
            {
                return format;
            }
        }

        return availableFormats[0];
    }

    VkPresentModeKHR SwapChain::choosePresentMode(std::vector<VkPresentModeKHR> &availablePresentModes)
    {
        for (const VkPresentModeKHR &presentMode : availablePresentModes)
        {
            if (presentMode == VK_PRESENT_MODE_MAILBOX_KHR)
            {
                return presentMode;
            }
        }

        return VK_PRESENT_MODE_FIFO_KHR;
    }
} // namespace vk