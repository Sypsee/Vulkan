#pragma once

#include "Window.h"
#include "../Vulkan/Device.h"
#include "../Vulkan/SwapChain.h"

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
    Window m_Window{START_WIDTH, START_HEIGHT, "Vulkan V2"};
    vk::Device m_Device{m_Window};
    vk::SwapChain m_SwapChain{m_Device, VkExtent2D{START_WIDTH, START_HEIGHT}};
};