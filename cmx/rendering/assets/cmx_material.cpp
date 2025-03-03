#include "cmx_material.h"
#include "cmx_frame_info.h"
#include "cmx_pipeline.h"
#include "cmx_render_system.h"
#include "cmx_renderer.h"
#include <vulkan/vulkan_enums.hpp>
#include <vulkan/vulkan_handles.hpp>

namespace cmx
{

size_t Material::_idProvider{0};
size_t Material::_boundID{1};

Material::Material(std::string vertPath, std::string fragPath)
    : _vertFilepath{vertPath}, _fragFilepath{fragPath}, _id{_idProvider}
{
    _renderSystem = RenderSystem::getInstance();

    _idProvider += 1;
    _boundID = _idProvider + 1;
}

void Material::editor()
{
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

    createPipelineLayout({});
    createPipeline(renderSystem->getRenderer()->getSwapChainRenderPass());
}

void Material::free()
{
    _pipeline->free();

    RenderSystem::getInstance()->getDevice()->device().destroyPipelineLayout(_pipelineLayout);
}

} // namespace cmx
