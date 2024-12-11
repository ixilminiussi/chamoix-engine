#pragma once

#include "cmx_buffer.h"
#include "cmx_device.h"
#include "tinyxml2.h"

// lib
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

// std
#include <memory>

namespace cmx
{

class CmxModel
{
  public:
    struct Vertex
    {
        glm::vec3 position;
        glm::vec3 color;
        glm::vec3 normal;
        glm::vec2 uv;

        static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
        static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();

        bool operator==(const Vertex &other) const
        {
            return position == other.position && color == other.color && normal == other.normal && uv == other.uv;
        }
    };
    struct Builder
    {
        std::vector<Vertex> vertices{};
        std::vector<uint32_t> indices{};
        std::string filepath;

        void loadModel(const std::string &filepath);
    };

    CmxModel(CmxDevice &, const CmxModel::Builder &, const std::string &name);
    ~CmxModel() = default;

    CmxModel(const CmxModel &) = delete;
    CmxModel &operator=(const CmxModel &) = delete;

    tinyxml2::XMLElement &save(tinyxml2::XMLDocument &doc, tinyxml2::XMLElement *parentComponent);

    static std::shared_ptr<CmxModel> createModelFromFile(CmxDevice &device, const std::string &filepath,
                                                         const std::string &name);

    void bind(VkCommandBuffer);
    void draw(VkCommandBuffer);

    const std::string name;

  private:
    void createVertexBuffers(const std::vector<Vertex> &);
    void createIndexBuffers(const std::vector<uint32_t> &);

    CmxDevice &cmxDevice;

    std::unique_ptr<CmxBuffer> vertexBuffer;
    uint32_t vertexCount;

    bool hasIndexBuffer{false};
    std::unique_ptr<CmxBuffer> indexBuffer;
    uint32_t indexCount;

    std::string filepath;
};

} // namespace cmx
