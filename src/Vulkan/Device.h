#pragma once

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <vector>
#include <optional>

#include "../Core/Window.h"

namespace vk
{
    struct QueueFamilyIndices {
        std::optional<uint32_t> graphicsFamily;
        std::optional<uint32_t> presentFamily;

        bool isComplete() {
            return graphicsFamily.has_value() && presentFamily.has_value();
        }
    };

    struct SwapChainSupportDetails {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;
    };

    class Device
    {
    public:
        Device(Window &window);
        ~Device();

        Device(const Device &) = delete;
        Device operator=(const Device &) = delete;

        SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);
        QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);

        const inline VkDevice getDevice() const { return m_VkDevice; }
        const inline VkPhysicalDevice getPhysicalDevice() const { return m_VkPhysicalDevice; }
        const inline VkSurfaceKHR getSurface() const { return m_VkSurface; }
        const inline VkQueue getGraphicsQueue() const { return m_VkGraphicsQueue; }
        const inline VkQueue getPresentQueue() const { return m_VkPresentQueue; }

    private:
        std::vector<const char*> getRequiredExtensions();
        void createInstance();
        void setupDebugMessenger();
        void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &createInfo);
        void createSurface();
        bool checkDeviceExtensionSupport(VkPhysicalDevice device);
        bool isDeviceSuitable(VkPhysicalDevice device);
        void pickPhysicalDevice();
        void createLogicalDevice();
        bool checkValidationLayerSupport();

        VkInstance m_VkInstance = VK_NULL_HANDLE;
        VkPhysicalDevice m_VkPhysicalDevice = VK_NULL_HANDLE;
        VkDevice m_VkDevice = VK_NULL_HANDLE;
        VkQueue m_VkGraphicsQueue = VK_NULL_HANDLE;
        VkQueue m_VkPresentQueue = VK_NULL_HANDLE;
        VkDebugUtilsMessengerEXT m_VkDebugMessenger = VK_NULL_HANDLE;

        VkSurfaceKHR m_VkSurface;
        Window &m_Window;

        const std::vector<const char*> validationLayers = {
            "VK_LAYER_KHRONOS_validation"
        };

        const std::vector<const char*> deviceExtensions = {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME
        };

        #ifdef NDEBUG
            const bool enableValidationLayers = false;
        #else
            const bool enableValidationLayers = true;
        #endif
    };
}
