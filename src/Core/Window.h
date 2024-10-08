#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>

class Window
{
public:
    Window(int width, int height, const char *title);
    ~Window();

    Window(const Window &) = delete;
    Window operator=(const Window &) = delete;

    inline bool shouldClose() { return glfwWindowShouldClose(m_Window); }
    inline VkExtent2D getExtent() { return {static_cast<uint32_t>(m_Width), static_cast<uint32_t>(m_Height)}; }
    inline bool wasWindowResized() { return m_FramebufferResized; }
    inline void resetWindowResizeFlag() { m_FramebufferResized = false; }
    inline GLFWwindow *getGLFWwindow() const { return m_Window; }

    void createWindowSurface(VkInstance instance, VkSurfaceKHR *surface);
    void setExtent(VkExtent2D extent);

private:
    static void framebufferResizeCallback(GLFWwindow *window, int width, int height);

    GLFWwindow *m_Window;
    int m_Width, m_Height;
    bool m_FramebufferResized = false;
};
