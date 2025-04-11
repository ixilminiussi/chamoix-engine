#ifndef CMX_MODEL
#define CMX_MODEL

#include "tinyxml2.h"

// lib
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <vulkan/vulkan.hpp>

// std
#include <memory>
#include <string>
#include <vector>

namespace cmx
{

class Model
{
  public:
    struct Vertex
    {
        glm::vec3 position;
        glm::vec3 color;
        glm::vec3 normal;
        glm::vec2 uv;
        glm::vec3 tangent;

        static std::vector<vk::VertexInputBindingDescription> getBindingDescriptions();
        static std::vector<vk::VertexInputAttributeDescription> getAttributeDescriptions();

        bool operator==(const Vertex &other) const
        {
            return position == other.position && color == other.color && normal == other.normal && uv == other.uv &&
                   tangent == other.tangent;
        }
    };
    struct Builder
    {
        std::vector<Vertex> vertices{};
        std::vector<uint32_t> indices{};
        std::string filepath;

        void loadModel(const std::string &filepath);
    };

    Model(class Device *, const Model::Builder &, const std::string &name);
    ~Model();

    void free();

    Model(const Model &) = delete;
    Model &operator=(const Model &) = delete;

    tinyxml2::XMLElement &save(tinyxml2::XMLDocument &doc, tinyxml2::XMLElement *parentElement);

    static Model *createModelFromFile(class Device *, const std::string &filepath, const std::string &name);

    void bind(vk::CommandBuffer);
    void draw(vk::CommandBuffer);

    void editor();

    const std::string name;

  private:
    void createVertexBuffers(class Device *, const std::vector<Vertex> &);
    void createIndexBuffers(class Device *, const std::vector<uint32_t> &);

    std::unique_ptr<class Buffer> _vertexBuffer;
    uint32_t _vertexCount;

    bool _hasIndexBuffer{false};
    std::unique_ptr<class Buffer> _indexBuffer;
    uint32_t _indexCount;

    std::string _filepath;

    bool _freed{false};
};

} // namespace cmx

#endif
