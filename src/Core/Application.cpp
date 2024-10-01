#include "Application.h"

// std
#include <thread>
#include <time.h>

Application::Application()
{   
}

Application::~Application()
{
}

void Application::run()
{
    while (!m_Window.shouldClose())
    {
        glfwPollEvents();
            
        std::this_thread::sleep_for(std::chrono::milliseconds(33));
    }
}