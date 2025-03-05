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

Material::Material(const std::string &vertPath, const std::string &fragPath, bool modelBased)
    : _vertFilepath{vertPath}, _fragFilepath{fragPath}, _modelBased{modelBased}, _id{_idProvider}
{
    _renderSystem = RenderSystem::getInstance();

    _idProvider += 1;
    _boundID = _idProvider + 1;
}

Material::Material(const std::string &vertPath, const std::string &fragPath, size_t id, bool modelBased)
    : _vertFilepath{vertPath}, _fragFilepath{fragPath}, _modelBased{modelBased}, _id{id}
{
    _renderSystem = RenderSystem::getInstance();
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

Material::Material(size_t ID) : _id(ID)
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

    _bindings.clear();
}

void Material::loadBindings()
{
    _bindings.clear();

    auto vertBindings = loadBindings(_vertFilepath);
    spdlog::info("Material: {0} loaded with {1} bindings", _vertFilepath, vertBindings.size());

    for (const SpvReflectDescriptorBinding *binding : vertBindings)
    {
        spdlog::info("Adding vert binding: set = {0}, binding = {1}, type = {2}", binding->set, binding->binding,
                     (int)binding->descriptor_type);
        _bindings.emplace(binding->set, binding->binding, binding->descriptor_type);
    }

    auto fragBindings = loadBindings(_fragFilepath);
    spdlog::info("Material: {0} loaded with {1} bindings", _fragFilepath, fragBindings.size());

    for (const SpvReflectDescriptorBinding *binding : fragBindings)
    {
        spdlog::info("Adding frag binding: set = {0}, binding = {1}, type = {2}", binding->set, binding->binding,
                     (int)binding->descriptor_type);
        _bindings.emplace(binding->set, binding->binding, binding->descriptor_type);
    }

    spdlog::info("Material: {0}, {1} loaded with {2} bindings", _vertFilepath.c_str(), _fragFilepath.c_str(),
                 _bindings.size());
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
        throw std::runtime_error(std::string("Material: failed to load bindings for ") + filename);
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
