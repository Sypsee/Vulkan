#include <vulkan/vulkan.h>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <vector>
#include <array>

#include "../Vulkan/Device.h"

class Model
{
public:
    struct Vertex
    {
        glm::vec2 pos;
        glm::vec3 color;

        inline static VkVertexInputBindingDescription getBindingDescription() {
            VkVertexInputBindingDescription bindingDescription{};
            bindingDescription.binding = 0;
            bindingDescription.stride = sizeof(Vertex);
            bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

            return bindingDescription;
        }

        inline static std::array<VkVertexInputAttributeDescription, 2> getAttributeDescriptions() {
            std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions{};
            attributeDescriptions[0].binding = 0;
            attributeDescriptions[0].location = 0;
            attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
            attributeDescriptions[0].offset = offsetof(Vertex, pos);

            attributeDescriptions[1].binding = 0;
            attributeDescriptions[1].location = 1;
            attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
            attributeDescriptions[1].offset = offsetof(Vertex, color);

            return attributeDescriptions;
        }
    };

    Model(vk::Device &device, const std::vector<Vertex> &vertices);
    ~Model();

    void bind(VkCommandBuffer commandBuffer);
    void draw(VkCommandBuffer commandBuffer);

private:
    vk::Device &m_Device;
    VkBuffer m_VertexBuffer;
    VkDeviceMemory m_VertexBufferMemory;
    uint32_t vertexCount=0;
};
