#pragma once

#include <vulkan/vulkan.h>

#include "Device.h"

// std
#include <iostream>
#include <vector>
#include <string>
#include <fstream>

namespace vk
{
    class Pipeline
    {
    public:
        Pipeline(Device &deviceRef, VkRenderPass renderPass, VkExtent2D deviceExtent);
        ~Pipeline();

        VkPipeline getGraphicsPipeline() const {return m_VkGraphicsPipeline;}
        void bind(const VkCommandBuffer& commandBuffer);

    private:
        void init();

        VkShaderModule createShaderModule(const std::vector<char>& code);
        std::vector<char> readFile(const std::string& fileName);

        Device &m_Device;
        VkExtent2D m_DeviceExtent;
        VkRenderPass m_VkRenderPass;
        VkPipelineLayout m_VkPipelineLayout;
        VkPipeline m_VkGraphicsPipeline;
    };
}
