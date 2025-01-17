#ifndef CMX_MODEL
#define CMX_MODEL

#include "tinyxml2.h"

// lib
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <vulkan/vulkan_core.h>

// std
#include <memory>
#include <string>
#include <vector>

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

    CmxModel(class CmxDevice *, const CmxModel::Builder &, const std::string &name);
    ~CmxModel();

    CmxModel(const CmxModel &) = delete;
    CmxModel &operator=(const CmxModel &) = delete;

    tinyxml2::XMLElement &save(tinyxml2::XMLDocument &doc, tinyxml2::XMLElement *parentElement);

    static CmxModel *createModelFromFile(class CmxDevice *, const std::string &filepath, const std::string &name);

    void bind(VkCommandBuffer);
    void draw(VkCommandBuffer);

    void free();

    const std::string name;

  private:
    void createVertexBuffers(class CmxDevice *, const std::vector<Vertex> &);
    void createIndexBuffers(class CmxDevice *, const std::vector<uint32_t> &);

    std::unique_ptr<class CmxBuffer> _vertexBuffer;
    uint32_t _vertexCount;

    bool _hasIndexBuffer{false};
    std::unique_ptr<class CmxBuffer> _indexBuffer;
    uint32_t _indexCount;

    std::string _filepath;

    bool _freed{false};
};

} // namespace cmx

#endif
