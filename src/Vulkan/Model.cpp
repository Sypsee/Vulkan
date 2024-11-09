#include "Model.h"

#include <cassert>
#include <cstring>

uint32_t findMemoryTypeIndex(uint32_t typeFilter, VkPhysicalDeviceMemoryProperties memProperties, VkMemoryPropertyFlags properties)
{
    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }

    throw std::runtime_error("failed to find suitable memory type!");
}

Model::Model(vk::Device &device, const std::vector<Vertex> &vertices)
    :m_Device(device), vertexCount(vertices.size())
{
    assert(vertices.size() >= 3 && "Vertex count must be atleast 3!");

    VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();
    
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = bufferSize;
    bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(m_Device.getDevice(), &bufferInfo, nullptr, &m_VertexBuffer) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create vertex buffer!\n");
    }

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(m_Device.getDevice(), m_VertexBuffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(m_Device.getPhysicalDevice(), &memProperties);
    allocInfo.memoryTypeIndex = findMemoryTypeIndex(memRequirements.memoryTypeBits, memProperties, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    if (vkAllocateMemory(m_Device.getDevice(), &allocInfo, nullptr, &m_VertexBufferMemory) != VK_SUCCESS) {
        throw std::runtime_error("Failed to allocate vertex buffer memory!\n");
    }

    vkBindBufferMemory(m_Device.getDevice(), m_VertexBuffer, m_VertexBufferMemory, 0);

    void *data;
    vkMapMemory(m_Device.getDevice(), m_VertexBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, vertices.data(), static_cast<size_t>(bufferSize));
    vkUnmapMemory(m_Device.getDevice(), m_VertexBufferMemory);
}

Model::~Model()
{
    vkDestroyBuffer(m_Device.getDevice(), m_VertexBuffer, nullptr);
    vkFreeMemory(m_Device.getDevice(), m_VertexBufferMemory, nullptr);
}

void Model::bind(VkCommandBuffer commandBuffer)
{
    VkBuffer buffers[] = {m_VertexBuffer};
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);
}

void Model::draw(VkCommandBuffer commandBuffer)
{
    vkCmdDraw(commandBuffer, vertexCount, 1, 0, 0);
}
