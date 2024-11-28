#pragma once

#include "cmx_device.h"

// lib
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

namespace cmx
{

class CmxModel
{
  public:
    struct Vertex
    {
        glm::vec3 position;
        glm::vec3 color;

        static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
        static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();
    };

    CmxModel(CmxDevice &, const std::vector<Vertex> &);
    ~CmxModel();

    CmxModel(const CmxModel &) = delete;
    CmxModel &operator=(const CmxModel &) = delete;

    void bind(VkCommandBuffer);
    void draw(VkCommandBuffer);

  private:
    void createVertexBuffers(const std::vector<Vertex> &);

    CmxDevice &cmxDevice;
    VkBuffer vertexBuffer;
    VkDeviceMemory vertexBufferMemory;
    uint32_t vertexCount;
};

} // namespace cmx
