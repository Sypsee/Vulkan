#pragma once

#include "Window.h"
#include "../Vulkan/Device.h"

class Application
{
public:
    const int START_WIDTH = 800;
    const int START_HEIGHT = 600;

    Application();
    ~Application();

    Application(const Application &) = delete;
    Application operator=(const Application &) = delete;

    void run();

private:
    Window m_Window{START_WIDTH, START_HEIGHT, "Vulkan V2"};
    Device m_Device{m_Window};
};