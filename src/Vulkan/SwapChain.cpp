#include "SwapChain.h"

namespace vk
{
    SwapChain::SwapChain(Device &deviceRef, VkExtent2D deviceExtent)
        :m_Device(deviceRef), m_DeviceExtent(deviceExtent)
    {
        init();
    }
    
    SwapChain::SwapChain(Device &deviceRef, VkExtent2D deviceExtent, std::shared_ptr<SwapChain> prev)
        :m_Device(deviceRef), m_DeviceExtent(deviceExtent), m_OldSwapChain(prev)
    {
        init();
        m_OldSwapChain = nullptr;
    }

    SwapChain::~SwapChain()
    {
        for (VkImageView &imageView : m_SwapChainImageViews)
        {
            vkDestroyImageView(m_Device.getDevice(), imageView, NULL);
        }
        
        m_SwapChainImageViews.clear();

        vkDestroySwapchainKHR(m_Device.getDevice(), m_VkSwapChain, NULL);

        for (auto framebuffer : m_SwapChainFramebuffers) {
            vkDestroyFramebuffer(m_Device.getDevice(), framebuffer, nullptr);
        }

        m_SwapChainFramebuffers.clear();

        vkDestroyRenderPass(m_Device.getDevice(), m_VkRenderPass, nullptr);

        for (int i=0; i<MAX_FRAMES_IN_FLIGHT; i++)
        {
            vkDestroySemaphore(m_Device.getDevice(), m_ImageAvailableSemaphores[i], nullptr);
            vkDestroySemaphore(m_Device.getDevice(), m_RenderFinishedSemaphores[i], nullptr);
            vkDestroyFence(m_Device.getDevice(), m_InFlightFences[i], nullptr);
        }
    }

    void SwapChain::init()
    {
        createSwapChain();
        createImageViews();
        createRenderPass();
        createFramebuffers();
        createSyncObjects();
    }

    VkResult SwapChain::acquireNextImage(uint32_t *imageIndex)
    {
        if (m_InFlightFences[m_CurrentFrame] != VK_NULL_HANDLE)
        {
            if (vkWaitForFences(m_Device.getDevice(), 1, &m_InFlightFences[m_CurrentFrame], VK_TRUE, UINT64_MAX) != VK_SUCCESS)
            {
                throw std::runtime_error("Failed waiting on fence! (acquireNextImage) \n");
            }
        }

        VkResult res = vkAcquireNextImageKHR(m_Device.getDevice(), m_VkSwapChain, UINT64_MAX, m_ImageAvailableSemaphores[m_CurrentFrame], VK_NULL_HANDLE, imageIndex);
        return res;
    }

    VkResult SwapChain::submitCommandBuffers(const VkCommandBuffer *buffers, uint32_t *imageIndex)
    {
        // if (m_ImagesInFlight[*imageIndex] != VK_NULL_HANDLE)
        // {
        //     if (vkWaitForFences(m_Device.getDevice(), 1, &m_ImagesInFlight[*imageIndex], VK_TRUE, UINT64_MAX) != VK_SUCCESS)
        //     {
        //         throw std::runtime_error("Failed waiting on fence! (submitCommandBuffers) \n");
        //     }
        // }
 
        m_ImagesInFlight[*imageIndex] = m_InFlightFences[m_CurrentFrame];

        VkSubmitInfo submitInfo = {};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        VkSemaphore waitSemaphores[] = {m_ImageAvailableSemaphores[m_CurrentFrame]};
        VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;

        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = buffers;

        VkSemaphore signalSemaphores[] = {m_RenderFinishedSemaphores[m_CurrentFrame]};
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;

        vkResetFences(m_Device.getDevice(), 1, &m_InFlightFences[m_CurrentFrame]);
        if (vkQueueSubmit(m_Device.getGraphicsQueue(), 1, &submitInfo, m_InFlightFences[m_CurrentFrame]) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to submit draw command buffer!\n");
        }

        VkPresentInfoKHR presentInfo = {};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = signalSemaphores;

        VkSwapchainKHR swapChains[] = {m_VkSwapChain};
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapChains;

        presentInfo.pImageIndices = imageIndex;

        VkResult result = vkQueuePresentKHR(m_Device.getPresentQueue(), &presentInfo);

        m_CurrentFrame = (m_CurrentFrame + 1) % MAX_FRAMES_IN_FLIGHT;

        return result;
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
        createInfo.oldSwapchain = m_OldSwapChain == nullptr ? VK_NULL_HANDLE : m_OldSwapChain->getSwapChain();

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

        VkResult res = vkCreateSwapchainKHR(m_Device.getDevice(), &createInfo, NULL, &m_VkSwapChain);
        if (res != VK_SUCCESS)
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

    void SwapChain::createRenderPass()
    {
        VkAttachmentDescription colorAttachment{};
        colorAttachment.format = m_SwapChainImageFormat;
        colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        VkAttachmentReference colorAttachmentRef{};
        colorAttachmentRef.attachment = 0;
        colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass{};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &colorAttachmentRef;

        VkSubpassDependency dependency{};
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass = 0;
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.srcAccessMask = 0;
        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

        VkRenderPassCreateInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = 1;
        renderPassInfo.pAttachments = &colorAttachment;
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpass;
        renderPassInfo.dependencyCount = 1;
        renderPassInfo.pDependencies = &dependency;

        if (vkCreateRenderPass(m_Device.getDevice(), &renderPassInfo, nullptr, &m_VkRenderPass) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create render pass!\n");
        }
    }

    void SwapChain::createFramebuffers()
    {
        m_SwapChainFramebuffers.resize(m_SwapChainImageViews.size());
        
        for (size_t i = 0; i < m_SwapChainImageViews.size(); i++)
        {
            VkImageView attachments[] = { m_SwapChainImageViews[i] };

            VkFramebufferCreateInfo framebufferInfo{};
            framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferInfo.renderPass = m_VkRenderPass;
            framebufferInfo.attachmentCount = 1;
            framebufferInfo.pAttachments = attachments;
            framebufferInfo.width = m_DeviceExtent.width;
            framebufferInfo.height = m_DeviceExtent.height;
            framebufferInfo.layers = 1;    

            if (vkCreateFramebuffer(m_Device.getDevice(), &framebufferInfo, nullptr, &m_SwapChainFramebuffers[i]) != VK_SUCCESS) {
                throw std::runtime_error("Failed to create framebuffer!\n");
            }
        }
    }

    void SwapChain::createSyncObjects()
    {
        m_ImageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        m_RenderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        m_InFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
        m_ImagesInFlight.resize(MAX_FRAMES_IN_FLIGHT);

        VkSemaphoreCreateInfo semaphoreInfo{};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkFenceCreateInfo fenceInfo{};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
        {
            if (vkCreateSemaphore(m_Device.getDevice(), &semaphoreInfo, nullptr, &m_ImageAvailableSemaphores[i]) != VK_SUCCESS ||
                vkCreateSemaphore(m_Device.getDevice(), &semaphoreInfo, nullptr, &m_RenderFinishedSemaphores[i]) != VK_SUCCESS ||
                vkCreateFence(m_Device.getDevice(), &fenceInfo, nullptr, &m_InFlightFences[i]) != VK_SUCCESS)
            {
                throw std::runtime_error("Failed to create sync objects!\n");
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
