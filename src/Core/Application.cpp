#include "Application.h"

// std
#include <thread>

Application::Application()
{
    recreateSwapChain();
    createCommandPool();
    createCommandBuffer();
    loadModels();
}

Application::~Application()
{
    vkDestroyCommandPool(m_Device.getDevice(), m_VkCommandPool, nullptr);
}

void Application::createCommandPool()
{
    vk::QueueFamilyIndices queueFamilyIndices = m_Device.findQueueFamilies(m_Device.getPhysicalDevice());

    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

    if (vkCreateCommandPool(m_Device.getDevice(), &poolInfo, nullptr, &m_VkCommandPool) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create command pool!\n");
    }
}

void Application::createCommandBuffer()
{
    m_VkCommandBuffers.resize(m_SwapChain->MAX_FRAMES_IN_FLIGHT);

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = m_VkCommandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;

    for (int i = 0; i < m_SwapChain->MAX_FRAMES_IN_FLIGHT+1; i++)
    {
        if (vkAllocateCommandBuffers(m_Device.getDevice(), &allocInfo, &m_VkCommandBuffers[i]) != VK_SUCCESS) {
            throw std::runtime_error("Failed to allocate command buffers!\n");
        }
    }
}

void Application::freeCommandBuffer()
{
    vkFreeCommandBuffers(m_Device.getDevice(),
                         m_VkCommandPool,
                         m_VkCommandBuffers.size(),
                         m_VkCommandBuffers.data());
    m_VkCommandBuffers.clear();
}

void Application::recreateSwapChain()
{
    VkExtent2D extent = m_Window.getExtent();
    while (extent.width == 0 || extent.height == 0)
    {
        extent = m_Window.getExtent();
        glfwWaitEvents();
    }

    vkDeviceWaitIdle(m_Device.getDevice());
    if (m_SwapChain == nullptr)
    {
        m_SwapChain = std::make_unique<vk::SwapChain>(m_Device, extent);
    }
    else
    {
        m_SwapChain = std::make_unique<vk::SwapChain>(m_Device, extent, std::move(m_SwapChain));
        if (m_SwapChain->getImageCount() != m_VkCommandBuffers.size())
        {
            freeCommandBuffer();
            createCommandBuffer();
        }
    }
    createPipeline();
}

void Application::createPipeline()
{
    m_Pipeline = std::make_unique<vk::Pipeline>(m_Device, m_SwapChain->getRenderPass(), m_SwapChain->getDeviceExtent());
}

void Application::recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex)
{
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = 0; // Optional
    beginInfo.pInheritanceInfo = nullptr; // Optional

    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
        throw std::runtime_error("Failed to begin recording command buffer!\n");
    }

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = m_SwapChain->getRenderPass();
    renderPassInfo.framebuffer = m_SwapChain->getFramebuffer(imageIndex);
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = m_SwapChain->getDeviceExtent();
    VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &clearColor;

    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    m_Pipeline->bind(commandBuffer);

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(m_SwapChain->getDeviceExtent().width);
    viewport.height = static_cast<float>(m_SwapChain->getDeviceExtent().height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = m_SwapChain->getDeviceExtent();
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

    m_Model->bind(commandBuffer);
    m_Model->draw(commandBuffer);

    vkCmdEndRenderPass(commandBuffer);
    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
        throw std::runtime_error("Failed to record command buffer!\n");
    }
}

void Application::loadModels()
{
    const std::vector<Model::Vertex> vertices = {
        {{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
        {{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
        {{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}
    };

    m_Model = std::make_unique<Model>(m_Device, vertices);
}

void Application::run()
{
    while (!m_Window.shouldClose())
    {
        glfwPollEvents();
        drawFrame();
    
        std::this_thread::sleep_for(std::chrono::milliseconds(33));
    }
    
    vkDeviceWaitIdle(m_Device.getDevice());
}

void Application::drawFrame()
{
    uint32_t imageIndex;
    VkResult result = m_SwapChain->acquireNextImage(&imageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR)
    {
        recreateSwapChain();
        return;
    }

    if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
    {
        throw std::runtime_error("Failed to acquire next image!\n");
    }

    recordCommandBuffer(m_VkCommandBuffers[imageIndex], imageIndex);
    result = m_SwapChain->submitCommandBuffers(&m_VkCommandBuffers[imageIndex], &imageIndex);
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || m_Window.wasWindowResized())
    {
        m_Window.resetWindowResizeFlag();
        recreateSwapChain();
        return;
    }
    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to submit command buffer\n");
    }
}
