#include "Window.h"

Window::Window(int width, int height, const char *title)
    : m_Width(width), m_Height(height)
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); // doing it because we are not using OPENGL.
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);    // resizable

    m_Window = glfwCreateWindow(m_Width, m_Height, title, nullptr, nullptr);
    glfwSetWindowUserPointer(m_Window, this);
    glfwSetFramebufferSizeCallback(m_Window, framebufferResizeCallback);
}

Window::~Window()
{
    glfwDestroyWindow(m_Window);
    glfwTerminate();
}

void Window::createWindowSurface(VkInstance instance, VkSurfaceKHR *surface)
{
    if (glfwCreateWindowSurface(instance, m_Window, nullptr, surface) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create window surface!");
    }
}

void Window::framebufferResizeCallback(GLFWwindow *window, int width, int height)
{
    auto myWindow = reinterpret_cast<Window *>(glfwGetWindowUserPointer(window));
    myWindow->m_FramebufferResized = true;
    myWindow->m_Width = width;
    myWindow->m_Height = height;
}