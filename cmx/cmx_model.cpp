#include "cmx_model.h"

// cmx
#include "cmx_utils.h"

// lib
#include <spdlog/spdlog.h>
#include <vulkan/vulkan_core.h>
#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

// std
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <strings.h>
#include <unordered_map>

template <> struct std::hash<cmx::CmxModel::Vertex>
{
    size_t operator()(cmx::CmxModel::Vertex const &vertex) const
    {
        size_t seed = 0;
        cmx::hashCombine(seed, vertex.position, vertex.color, vertex.normal, vertex.uv);
        return seed;
    }
};

namespace cmx
{

CmxModel::CmxModel(CmxDevice &device, const CmxModel::Builder &builder, const std::string &name)
    : _cmxDevice{device}, name{name}
{
    createVertexBuffers(builder.vertices);
    createIndexBuffers(builder.indices);
    _filepath = builder.filepath;
}

std::shared_ptr<CmxModel> CmxModel::createModelFromFile(CmxDevice &device, const std::string &filepath,
                                                        const std::string &name)
{
    Builder builder{};
    builder.loadModel(filepath);

    spdlog::info("CmxModel: '{0}' loaded with {1} vertices", filepath, builder.vertices.size());
    return std::make_shared<CmxModel>(device, builder, name);
}

void CmxModel::bind(VkCommandBuffer commandBuffer)
{
    VkBuffer buffers[] = {_vertexBuffer->getBuffer()};
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);

    if (_hasIndexBuffer)
    {
        vkCmdBindIndexBuffer(commandBuffer, _indexBuffer->getBuffer(), 0, VK_INDEX_TYPE_UINT32);
    }
}

void CmxModel::draw(VkCommandBuffer commandBuffer)
{
    if (_hasIndexBuffer)
    {
        vkCmdDrawIndexed(commandBuffer, _indexCount, 1, 0, 0, 0);
    }
    else
    {
        vkCmdDraw(commandBuffer, _vertexCount, 1, 0, 0);
    }
}

void CmxModel::createVertexBuffers(const std::vector<Vertex> &vertices)
{
    _vertexCount = static_cast<uint32_t>(vertices.size());
    assert(_vertexCount >= 3 && "Vertex count must be at least 3");
    VkDeviceSize bufferSize = sizeof(vertices[0]) * _vertexCount;
    uint32_t vertexSize = sizeof(vertices[0]);

    CmxBuffer stagingBuffer{_cmxDevice, vertexSize, _vertexCount, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT};

    stagingBuffer.map();
    stagingBuffer.writeToBuffer((void *)vertices.data());

    _vertexBuffer = std::make_unique<CmxBuffer>(_cmxDevice, vertexSize, _vertexCount,
                                                VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                                                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    _cmxDevice.copyBuffer(stagingBuffer.getBuffer(), _vertexBuffer->getBuffer(), bufferSize);
}

void CmxModel::createIndexBuffers(const std::vector<uint32_t> &indices)
{
    _indexCount = static_cast<uint32_t>(indices.size());
    _hasIndexBuffer = _indexCount > 0;

    if (!_hasIndexBuffer)
        return;

    VkDeviceSize bufferSize = sizeof(indices[0]) * _indexCount;
    uint32_t indexSize = sizeof(indices[0]);

    CmxBuffer stagingBuffer{_cmxDevice, indexSize, _indexCount, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT};

    stagingBuffer.map();
    stagingBuffer.writeToBuffer((void *)indices.data());

    _indexBuffer = std::make_unique<CmxBuffer>(_cmxDevice, indexSize, _indexCount,
                                               VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                                               VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    _cmxDevice.copyBuffer(stagingBuffer.getBuffer(), _indexBuffer->getBuffer(), bufferSize);
}

std::vector<VkVertexInputBindingDescription> CmxModel::Vertex::getBindingDescriptions()
{
    std::vector<VkVertexInputBindingDescription> bindingDescriptions(1);
    bindingDescriptions[0].binding = 0;
    bindingDescriptions[0].stride = sizeof(Vertex);
    bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    return bindingDescriptions;
}

std::vector<VkVertexInputAttributeDescription> CmxModel::Vertex::getAttributeDescriptions()
{
    std::vector<VkVertexInputAttributeDescription> attributeDescriptions{};

    attributeDescriptions.push_back({0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, position)});
    attributeDescriptions.push_back({1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, color)});
    attributeDescriptions.push_back({2, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, normal)});
    attributeDescriptions.push_back({3, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, uv)});
    return attributeDescriptions;
}

void CmxModel::Builder::loadModel(const std::string &filepath)
{
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;

    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filepath.c_str()))
    {
        spdlog::warn("tinyobj: {0}", warn);
        spdlog::error("tinyobj: {1}", err);
        std::exit(EXIT_FAILURE);
    }

    vertices.clear();
    indices.clear();

    std::unordered_map<Vertex, uint32_t> uniqueVertices{};
    for (const auto &shape : shapes)
    {
        for (const auto &index : shape.mesh.indices)
        {
            Vertex vertex{};

            if (index.vertex_index >= 0)
            {
                vertex.position = {
                    attrib.vertices[3 * index.vertex_index + 0],
                    attrib.vertices[3 * index.vertex_index + 1],
                    attrib.vertices[3 * index.vertex_index + 2],
                };

                vertex.color = {
                    attrib.colors[3 * index.vertex_index - 2],
                    attrib.colors[3 * index.vertex_index - 1],
                    attrib.colors[3 * index.vertex_index - 0],
                };
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

            if (uniqueVertices.count(vertex) == 0)
            {
                uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
                vertices.push_back(vertex);
            }
            indices.push_back(uniqueVertices[vertex]);
        }
    }
    this->filepath = filepath;
}

tinyxml2::XMLElement &CmxModel::save(tinyxml2::XMLDocument &doc, tinyxml2::XMLElement *parentElement)
{
    tinyxml2::XMLElement *modelElement = doc.NewElement("model");

    modelElement->SetAttribute("filepath", _filepath.c_str());
    parentElement->InsertEndChild(modelElement);

    return *modelElement;
}

} // namespace cmx
