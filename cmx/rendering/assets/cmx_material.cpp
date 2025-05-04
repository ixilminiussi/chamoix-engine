#include "cmx_material.h"

// cmx
#include "cmx_editor.h"
#include "cmx_pipeline.h"
#include "cmx_render_system.h"
#include "cmx_renderer.h"

// lib
#include <imgui.h>
#include <vulkan/vulkan_enums.hpp>
#include <vulkan/vulkan_handles.hpp>

// std
#include <cstdlib>
#include <fstream>
#ifndef _WIN32
#include <cxxabi.h>
#else
#include <regex>
#endif
#include <memory>

namespace cmx
{

size_t Material::_idProvider{1};
size_t Material::_boundID{1};

Material::Material(const std::string &vertPath, const std::string &fragPath, bool modelBased, Role role)
    : _vertFilepath{vertPath}, _fragFilepath{fragPath}, _modelBased{modelBased}, _role{role}, _id{_idProvider},
      _doNotSave{false}, _editorOnly{false}, _isVisible{true}
{
    _renderSystem = RenderSystem::getInstance();

    _idProvider += 1;
    _boundID = _idProvider + 1;
}

Material::Material(const std::string &vertPath, const std::string &fragPath, size_t id, bool modelBased, Role role,
                   bool editorOnly)
    : _vertFilepath{vertPath}, _fragFilepath{fragPath}, _modelBased{modelBased}, _role{role}, _id{id}, _doNotSave{true},
      _editorOnly{editorOnly}, _isVisible{true}
{
    _renderSystem = RenderSystem::getInstance();
}

void Material::editor()
{
    ImGui::Text("Material:");
    ImGui::Checkbox("visible", &_isVisible);
    ImGui::SameLine();
    ImGui::Checkbox("editor only", &_editorOnly);
}

tinyxml2::XMLElement *Material::save(tinyxml2::XMLDocument &doc, tinyxml2::XMLElement *parentElement) const
{
    if (_doNotSave)
        return nullptr;

    tinyxml2::XMLElement *materialElement;

    switch (_role)
    {
    case Role::ePostProcess:
        materialElement = doc.NewElement("postProcess");
        break;
    default:
        materialElement = doc.NewElement("material");
        break;
    }

    materialElement->SetAttribute("type", getType().c_str());
    materialElement->SetAttribute("vertex", _vertFilepath.c_str());
    materialElement->SetAttribute("fragment", _fragFilepath.c_str());
    materialElement->SetAttribute("editorOnly", _editorOnly);

    parentElement->InsertEndChild(materialElement);

    return materialElement;
}

void Material::load(tinyxml2::XMLElement *materialElement)
{
    _vertFilepath = materialElement->Attribute("vertex");
    _fragFilepath = materialElement->Attribute("fragment");

    _editorOnly = materialElement->BoolAttribute("editorOnly");
}

bool Material::isVisible() const
{
#ifndef NDEBUG
    if (!Editor::isActive())
    {
        return !_editorOnly;
    }
    else
    {
        return _isVisible;
    }
#endif
    return _isVisible && !_editorOnly;
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
    createPipeline(renderSystem->getRenderPass());
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
    _requestedSamplerCount = 0;

    loadBindings(_vertFilepath);
    loadBindings(_fragFilepath);

    spdlog::info("Material: <{}>, '{}', {}' loaded with {} bindings", name.c_str(), _vertFilepath.c_str(),
                 _fragFilepath.c_str(), _bindings.size());
}

void Material::loadBindings(const std::string &filename)
{
    std::vector<uint32_t> spirvCode = loadSpirvData(filename);

    SpvReflectShaderModule module{};
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

    for (const SpvReflectDescriptorBinding *binding : bindings)
    {
        if (_bindings.emplace(binding->set, binding->binding, binding->descriptor_type, binding->image.dim).second)
        {
            if (binding->descriptor_type == SPV_REFLECT_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)
            {
                _requestedSamplerCount++;
            }
        }
    }

    // Clean up
    spvReflectDestroyShaderModule(&module);
}

std::vector<uint32_t> Material::loadSpirvData(const std::string &filename)
{
    std::ifstream file{std::string(SHADER_FILES) + filename, std::ios::binary};
    if (!file.is_open())
    {
        throw std::runtime_error("Shader: Could not open file: " + filename);
    }
    file.seekg(0, std::ios::end);
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);
    std::vector<uint32_t> buffer(static_cast<size_t>(size) / sizeof(uint32_t));
    if (!file.read(reinterpret_cast<char *>(buffer.data()), size))
    {
        throw std::runtime_error("Error reading file: " + filename);
    }
    return buffer;
}

#ifdef _WIN32
std::string Material::getType() const
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
