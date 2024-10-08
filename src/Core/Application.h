#pragma once

#include "Window.h"
#include "../Vulkan/Device.h"
#include "../Vulkan/SwapChain.h"
#include "../Vulkan/Pipeline.h"

class Application
{
public:
    const uint16_t START_WIDTH = 800;
    const uint16_t START_HEIGHT = 600;

    Application();
    ~Application();

    Application(const Application &) = delete;
    Application operator=(const Application &) = delete;

    void run();

private:
    void createCommandPool();
    void createCommandBuffer();
    void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);
    void drawFrame();
    void createPipeline();
    void recreateSwapChain();

    Window m_Window{START_WIDTH, START_HEIGHT, "Vulkan V2"};
    vk::Device m_Device{m_Window};
    std::unique_ptr<vk::SwapChain> m_SwapChain;
    std::unique_ptr<vk::Pipeline> m_Pipeline;

    VkCommandPool m_VkCommandPool;
    std::vector<VkCommandBuffer> m_VkCommandBuffers;
};
