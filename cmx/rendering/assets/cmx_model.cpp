#include "cmx_model.h"

// cmx
#include "cmx_buffer.h"
#include "cmx_physics.h"
#include "cmx_utils.h"

// lib
#include <cstddef>
#include <spdlog/spdlog.h>
#include <stdexcept>
#include <vulkan/vulkan_core.h>
#include <vulkan/vulkan_enums.hpp>
#include <vulkan/vulkan_handles.hpp>
#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

// std
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <unordered_map>

template <> struct std::hash<cmx::Model::Vertex>
{
    size_t operator()(cmx::Model::Vertex const &vertex) const
    {
        size_t seed = 0;
        cmx::hashCombine(seed, vertex.position, vertex.color, vertex.normal, vertex.uv, vertex.tangent);
        return seed;
    }
};

namespace cmx
{

Model::Model(Device *device, const Model::Builder &builder, const std::string &name) : name{name}
{
    createVertexBuffers(device, builder.vertices);
    createIndexBuffers(device, builder.indices);
    _filepath = builder.filepath;
}

Model::~Model()
{
    if (!_freed)
    {
        spdlog::error("Model: forgot to free model {0} before deletion", name);
    }
}

void Model::free()
{
    _vertexBuffer->free();
    _indexBuffer->free();

    _freed = true;
}

Model *Model::createModelFromFile(Device *device, const std::string &filepath, const std::string &name)
{
    Builder builder{};
    builder.loadModel(filepath);

    spdlog::info("Model: '{0}' loaded with {1} vertices", filepath, builder.vertices.size());
    return new Model(device, builder, name);
}

void Model::bind(vk::CommandBuffer commandBuffer)
{
    vk::Buffer buffers[] = {_vertexBuffer->getBuffer()};
    vk::DeviceSize offsets[] = {0};
    commandBuffer.bindVertexBuffers(0, 1, buffers, offsets);

    if (_hasIndexBuffer)
    {
        commandBuffer.bindIndexBuffer(_indexBuffer->getBuffer(), 0, vk::IndexType::eUint32);
    }
}

void Model::draw(vk::CommandBuffer commandBuffer)
{
    if (_hasIndexBuffer)
    {
        commandBuffer.drawIndexed(_indexCount, 1, 0, 0, 0);
    }
    else
    {
        commandBuffer.draw(_vertexCount, 1, 0, 0);
    }
}

void Model::createVertexBuffers(Device *device, const std::vector<Vertex> &vertices)
{
    if (!device)
        return;

    _vertexCount = static_cast<uint32_t>(vertices.size());
    assert(_vertexCount >= 3 && "Vertex count must be at least 3");
    vk::DeviceSize bufferSize = sizeof(vertices[0]) * _vertexCount;
    uint32_t vertexSize = sizeof(vertices[0]);

    Buffer stagingBuffer{*device, vertexSize, _vertexCount, vk::BufferUsageFlagBits::eTransferSrc,
                         vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent};

    stagingBuffer.map();
    stagingBuffer.writeToBuffer((void *)vertices.data());

    _vertexBuffer =
        std::make_unique<Buffer>(*device, vertexSize, _vertexCount,
                                 vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst,
                                 vk::MemoryPropertyFlagBits::eDeviceLocal);

    device->copyBuffer(stagingBuffer.getBuffer(), _vertexBuffer->getBuffer(), bufferSize);
    stagingBuffer.free();
}

void Model::createIndexBuffers(Device *device, const std::vector<uint32_t> &indices)
{
    if (!device)
        return;

    _indexCount = static_cast<uint32_t>(indices.size());
    _hasIndexBuffer = _indexCount > 0;

    if (!_hasIndexBuffer)
        return;

    vk::DeviceSize bufferSize = sizeof(indices[0]) * _indexCount;
    uint32_t indexSize = sizeof(indices[0]);

    Buffer stagingBuffer{*device, indexSize, _indexCount, vk::BufferUsageFlagBits::eTransferSrc,
                         vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent};

    stagingBuffer.map();
    stagingBuffer.writeToBuffer((void *)indices.data());

    _indexBuffer = std::make_unique<Buffer>(
        *device, indexSize, _indexCount, vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst,
        vk::MemoryPropertyFlagBits::eDeviceLocal);

    device->copyBuffer(stagingBuffer.getBuffer(), _indexBuffer->getBuffer(), bufferSize);
    stagingBuffer.free();
}

std::vector<vk::VertexInputBindingDescription> Model::Vertex::getBindingDescriptions()
{
    std::vector<vk::VertexInputBindingDescription> bindingDescriptions(1);
    bindingDescriptions[0].binding = 0;
    bindingDescriptions[0].stride = sizeof(Vertex);
    bindingDescriptions[0].inputRate = vk::VertexInputRate::eVertex;
    return bindingDescriptions;
}

std::vector<vk::VertexInputAttributeDescription> Model::Vertex::getAttributeDescriptions()
{
    std::vector<vk::VertexInputAttributeDescription> attributeDescriptions{};

    attributeDescriptions.push_back({0, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, position)});
    attributeDescriptions.push_back({1, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, color)});
    attributeDescriptions.push_back({2, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, normal)});
    attributeDescriptions.push_back({3, 0, vk::Format::eR32G32Sfloat, offsetof(Vertex, uv)});
    attributeDescriptions.push_back({4, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, tangent)});
    return attributeDescriptions;
}

void Model::Builder::loadModel(const std::string &filepath)
{
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;

    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filepath.c_str()))
    {
        spdlog::warn("Model: tinyobj -> {0}", warn);
        throw std::runtime_error("Model: tinyobj ->" + err);
    }

    vertices.clear();
    indices.clear();

    std::unordered_map<Vertex, uint32_t> uniqueVertices{};
    for (const auto &shape : shapes)
    {
        for (int i = 0; i < shape.mesh.indices.size(); i += 3)
        {
            std::array<Vertex, 3> triVerts;

            for (int j = 0; j < 3; ++j)
            {
                const auto &index = shape.mesh.indices[i + j];
                Vertex vertex{};

                if (index.vertex_index >= 0)
                {
                    vertex.position = {
                        attrib.vertices[3 * index.vertex_index + 0],
                        attrib.vertices[3 * index.vertex_index + 1],
                        attrib.vertices[3 * index.vertex_index + 2],
                    };

                    if (!attrib.colors.empty())
                    {
                        vertex.color = {
                            attrib.colors[3 * index.vertex_index + 0],
                            attrib.colors[3 * index.vertex_index + 1],
                            attrib.colors[3 * index.vertex_index + 2],
                        };
                    }
                }

                if (index.normal_index >= 0)
                {
                    vertex.normal = {
                        attrib.normals[3 * index.normal_index + 0],
                        attrib.normals[3 * index.normal_index + 1],
                        attrib.normals[3 * index.normal_index + 2],
                    };
                }

                if (index.texcoord_index >= 0)
                {
                    vertex.uv = {
                        attrib.texcoords[2 * index.texcoord_index + 0],
                        attrib.texcoords[2 * index.texcoord_index + 1],
                    };
                }

                triVerts[j] = vertex;
            }

            // Compute tangent for the triangle
            glm::vec3 edge1 = triVerts[1].position - triVerts[0].position;
            glm::vec3 edge2 = triVerts[2].position - triVerts[0].position;

            glm::vec2 deltaUV1 = triVerts[1].uv - triVerts[0].uv;
            glm::vec2 deltaUV2 = triVerts[2].uv - triVerts[0].uv;

            float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

            glm::vec3 tangent = f * (edge1 * deltaUV2.y - edge2 * deltaUV1.y);

            for (int j = 0; j < 3; ++j)
            {
                Vertex &v = triVerts[j];
                if (uniqueVertices.count(v) == 0)
                {
                    v.tangent = tangent; // start with this triangle's tangent
                    uniqueVertices[v] = static_cast<uint32_t>(vertices.size());
                    vertices.push_back(v);
                }
                else
                {
                    uint32_t idx = uniqueVertices[v];
                    vertices[idx].tangent += tangent;
                }

                indices.push_back(uniqueVertices[v]);
            }
        }
        this->filepath = filepath;
    }
}

tinyxml2::XMLElement &Model::save(tinyxml2::XMLDocument &doc, tinyxml2::XMLElement *parentElement)
{
    tinyxml2::XMLElement *modelElement = doc.NewElement("model");

    modelElement->SetAttribute("filepath", _filepath.c_str());
    parentElement->InsertEndChild(modelElement);

    return *modelElement;
}

} // namespace cmx
