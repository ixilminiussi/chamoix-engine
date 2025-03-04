#include "cmx_material.h"

// cmx
#include ".external/spirv-reflect/spirv_reflect.h"
#include "cmx_pipeline.h"
#include "cmx_render_system.h"
#include "cmx_renderer.h"

// lib
#include <SPIRV-Reflect/spirv_reflect.h>
#include <cstdlib>
#include <vulkan/vulkan_enums.hpp>
#include <vulkan/vulkan_handles.hpp>

// std
#include <fstream>
#ifndef _WIN32
#include <cxxabi.h>
#endif
#include <memory>

namespace cmx
{

size_t Material::_idProvider{0};
size_t Material::_boundID{1};

Material::Material(std::string vertPath, std::string fragPath, bool modelBased)
    : _vertFilepath{vertPath}, _fragFilepath{fragPath}, _modelBased{modelBased}, _id{_idProvider}
{
    _renderSystem = RenderSystem::getInstance();

    _idProvider += 1;
    _boundID = _idProvider + 1;
}

void Material::editor()
{
}

tinyxml2::XMLElement &Material::save(tinyxml2::XMLDocument &doc, tinyxml2::XMLElement *parentElement) const
{
    tinyxml2::XMLElement *materialElement = doc.NewElement("material");

    materialElement->SetAttribute("type", getType().c_str());
    materialElement->SetAttribute("vertex", _vertFilepath.c_str());
    materialElement->SetAttribute("fragment", _fragFilepath.c_str());

    parentElement->InsertEndChild(materialElement);

    return *materialElement;
}

void Material::load(tinyxml2::XMLElement *materialElement)
{
    _vertFilepath = materialElement->Attribute("vertex");
    _fragFilepath = materialElement->Attribute("fragment");
}

void Material::resetBoundID()
{
    _boundID = _idProvider + 1;
}

Material::Material(int ID) : _id(ID)
{
}

void Material::initialize()
{
    RenderSystem *renderSystem = RenderSystem::getInstance();

    loadBindings();

    createPipelineLayout({});
    createPipeline(renderSystem->getRenderer()->getSwapChainRenderPass());
}

void Material::free()
{
    _pipeline->free();

    RenderSystem::getInstance()->getDevice()->device().destroyPipelineLayout(_pipelineLayout);
}

void Material::loadBindings()
{
    auto vertBindings = loadBindings(_vertFilepath);
    auto fragBindings = loadBindings(_fragFilepath);

    std::sort(vertBindings.begin(), vertBindings.end(),
              [](SpvReflectDescriptorBinding *a, SpvReflectDescriptorBinding *b) {
                  return std::tie(a->set, a->binding) < std::tie(b->set, b->binding);
              });
    std::sort(fragBindings.begin(), fragBindings.end(),
              [](SpvReflectDescriptorBinding *a, SpvReflectDescriptorBinding *b) {
                  return std::tie(a->set, a->binding) < std::tie(b->set, b->binding);
              });

    _bindings.clear();
    _bindings = std::vector<BindingInfo>{};
    _bindings.reserve(vertBindings.size() + fragBindings.size());

    auto vertIt = vertBindings.begin();
    auto fragIt = fragBindings.begin();

    while (vertIt != vertBindings.end() && fragIt != fragBindings.end())
    {
        SpvReflectDescriptorBinding *vertDescriptorBinding = (*vertIt);
        SpvReflectDescriptorBinding *fragDescriptorBinding = (*fragIt);

        BindingInfo info{};

        if (vertDescriptorBinding->set < fragDescriptorBinding->set || vertIt == vertBindings.end())
        {
            info.set = vertDescriptorBinding->set;
            info.binding = vertDescriptorBinding->binding;
            info.type = vertDescriptorBinding->descriptor_type;
            info.shaderStage = vk::ShaderStageFlagBits::eVertex;
            vertIt++;
            _bindings.push_back(info);
            continue;
        }
        if (vertDescriptorBinding->set > fragDescriptorBinding->set || fragIt == fragBindings.end())
        {
            info.set = fragDescriptorBinding->set;
            info.binding = fragDescriptorBinding->binding;
            info.type = fragDescriptorBinding->descriptor_type;
            info.shaderStage = vk::ShaderStageFlagBits::eVertex;
            fragIt++;
            _bindings.push_back(info);
            continue;
        }
        if (vertDescriptorBinding->set == fragDescriptorBinding->set)
        {
            if (vertDescriptorBinding->binding < fragDescriptorBinding->binding)
            {
                info.set = vertDescriptorBinding->set;
                info.binding = vertDescriptorBinding->binding;
                info.type = vertDescriptorBinding->descriptor_type;
                info.shaderStage = vk::ShaderStageFlagBits::eVertex;
                vertIt++;
                _bindings.push_back(info);
                continue;
            }
            if (vertDescriptorBinding->binding > fragDescriptorBinding->binding)
            {
                info.set = fragDescriptorBinding->set;
                info.binding = fragDescriptorBinding->binding;
                info.type = fragDescriptorBinding->descriptor_type;
                info.shaderStage = vk::ShaderStageFlagBits::eVertex;
                fragIt++;
                _bindings.push_back(info);
                continue;
            }
            if (vertDescriptorBinding->binding == fragDescriptorBinding->binding)
            {
                if (vertDescriptorBinding->descriptor_type == fragDescriptorBinding->descriptor_type)
                {
                    info.set = vertDescriptorBinding->set;
                    info.binding = vertDescriptorBinding->binding;
                    info.type = vertDescriptorBinding->descriptor_type;
                    info.shaderStage = vk::ShaderStageFlagBits::eAllGraphics;
                }
                else
                {
                    info.set = fragDescriptorBinding->set;
                    info.binding = fragDescriptorBinding->binding;
                    info.type = fragDescriptorBinding->descriptor_type;
                    info.shaderStage = vk::ShaderStageFlagBits::eFragment;

                    BindingInfo info2{};
                    info2.set = vertDescriptorBinding->set;
                    info2.binding = vertDescriptorBinding->binding;
                    info2.type = vertDescriptorBinding->descriptor_type;
                    info2.shaderStage = vk::ShaderStageFlagBits::eVertex;
                    _bindings.push_back(info2);
                }
                fragIt++;
                vertIt++;
                _bindings.push_back(info);
                continue;
            }
        }
    }
}

size_t Material::getTotalSamplers() const
{
    size_t count{0};
    for (const BindingInfo &info : _bindings)
    {
        if (info.type == SPV_REFLECT_DESCRIPTOR_TYPE_SAMPLER)
            count++;
    }
    return count;
}

std::vector<SpvReflectDescriptorBinding *> Material::loadBindings(const std::string &filename)
{
    std::vector<uint32_t> spirvCode = loadSpirvData(filename);

    SpvReflectShaderModule module;
    SpvReflectResult result =
        spvReflectCreateShaderModule(spirvCode.size() * sizeof(uint32_t), spirvCode.data(), &module);
    if (result != SPV_REFLECT_RESULT_SUCCESS)
    {
        spdlog::error("Material: failed to load bindings for {}", filename.c_str());
        std::exit(EXIT_FAILURE);
    }

    uint32_t bindingCount = 0;

    result = spvReflectEnumerateDescriptorBindings(&module, &bindingCount, nullptr);
    std::vector<SpvReflectDescriptorBinding *> bindings(bindingCount);
    result = spvReflectEnumerateDescriptorBindings(&module, &bindingCount, bindings.data());

    // Clean up
    spvReflectDestroyShaderModule(&module);

    return bindings;
}

std::vector<uint32_t> Material::loadSpirvData(const std::string &filename)
{
    std::ifstream file(filename, std::ios::binary | std::ios::ate);
    if (!file.is_open())
    {
        throw std::runtime_error("Shader: Could not open file: " + filename);
    }
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);
    std::vector<uint32_t> buffer(size / sizeof(uint32_t));
    if (!file.read(reinterpret_cast<char *>(buffer.data()), size))
    {
        throw std::runtime_error("Error reading file: " + filename);
    }
    return buffer;
}

#ifdef _WIN32
std::string Material : getType() const
{
    return std::regex_replace(typeid(*this).name(), std::regex(R"(^(class |struct ))"), "");
}
#else
std::string Material::getType() const
{
    int status;
    char *demangled = abi::__cxa_demangle(typeid(*this).name(), nullptr, nullptr, &status);

    if (status == 0)
    {
        std::string demangledString{demangled};
        std::free(demangled);
        return demangledString;
    }
    else
    {
        spdlog::critical("Material {0}: Error demangling component type", name);
        return typeid(this).name();
    }
}
#endif

} // namespace cmx
