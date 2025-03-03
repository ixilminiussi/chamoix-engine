#include "cmx_hud_component.h"

// cmx
#include "cmx_actor.h"
#include "cmx_assets_manager.h"
#include "cmx_texture.h"

// lib
#include <vulkan/vulkan_handles.hpp>

namespace cmx
{

HudComponent::HudComponent() : Drawable{&_parent}
{
}

HudComponent::~HudComponent() {};

void HudComponent::onAttach()
{
    AssetsManager *assetsManager = getScene()->getAssetsManager();
    setDrawOption({assetsManager->getMaterial("hud_material"), nullptr, {}});
}

// void HudComponent::render(const cmx::FrameInfo &frameInfo, vk::PipelineLayout pipelineLayout)
// {
//     if (getParent() == nullptr)
//     {
//         spdlog::critical("HudComponent <{0}>: _parent is expired", name.c_str());
//         return;
//     }
//
//     if (_texture == nullptr)
//     {
//         spdlog::error("HudComponent <{0}->{1}>: missing texture", getParent()->name.c_str(), name.c_str());
//         return;
//     }
//
//     _texture->bind(frameInfo.commandBuffer, pipelineLayout);
//
//     frameInfo.commandBuffer.draw(6, 1, 0, 0);
// }

// void HudComponent::setTexture(const std::string &name)
// {
//     if (getScene() != nullptr)
//     {
//         _texture = getScene()->getAssetsManager()->getTexture(name);
//     }
//     else
//     {
//         spdlog::error("HudComponent <{0}->{1}>: Cannot setTexture before attaching to Scene object",
//                       getParent()->name.c_str(), name.c_str());
//     }
// }
//
// std::string HudComponent::getTextureName() const
// {
//     if (_texture)
//     {
//         return _texture->name;
//     }
//     return "Missing texture";
// }

} // namespace cmx
