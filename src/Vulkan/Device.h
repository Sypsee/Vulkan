#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <vector>
#include <optional>

#include "../Core/Window.h"

struct QueueFamilyIndices {
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    bool isComplete() {
        return graphicsFamily.has_value() && presentFamily.has_value();
    }
};

class Device
{
public:
    Device(Window &window);
    ~Device();

    Device(const Device &) = delete;
    Device operator=(const Device &) = delete;

private:
    std::vector<const char*> getRequiredExtensions();
    void createInstance();
    void setupDebugMessenger();
    void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &createInfo);
    void createSurface();
    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
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

    #ifdef NDEBUG
        const bool enableValidationLayers = false;
    #else
        const bool enableValidationLayers = true;
    #endif
};