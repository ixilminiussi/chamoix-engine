#include "cmx_editor_render_system.h"

// cmx
#include "cmx_buffer.h"
#include "cmx_camera.h"
#include "cmx_component.h"
#include "cmx_descriptors.h"
#include "cmx_device.h"
#include "cmx_frame_info.h"
#include "cmx_graphics_manager.h"
#include "cmx_pipeline.h"
#include "cmx_render_system.h"
#include "cmx_renderer.h"
#include "cmx_window.h"

// lib
#include <GLFW/glfw3.h>
#include <cstdlib>
#include <glm/ext/scalar_constants.hpp>
#include <spdlog/common.h>
#include <spdlog/spdlog.h>
#include <vulkan/vulkan_core.h>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

// std
#include <memory>
#include <stdexcept>

namespace cmx
{

void EditorRenderSystem::initialize()
{
    spdlog::info("EditorRenderSystem: Successfully initialized!");
}

void EditorRenderSystem::render(const FrameInfo *frameInfo, std::vector<std::shared_ptr<Component>> &renderQueue,
                                class GraphicsManager *graphicsManager)
{
    auto it = renderQueue.begin();
    while (it != renderQueue.end())
    {
        auto renderComponent = *it;

        if (renderComponent->getRequestedRenderSystem() != EDITOR_RENDER_SYSTEM)
        {
            it = renderQueue.erase(it);
            graphicsManager->addToQueue(renderComponent);
            continue;
        }
        if (renderComponent->getVisible())
        {
            renderComponent->render(*frameInfo, _pipelineLayout);
        }
        it++;
    }
}

void EditorRenderSystem::createPipelineLayout(VkDescriptorSetLayout globalSetLayout) {};

void EditorRenderSystem::createPipeline(VkRenderPass renderPass) {};

} // namespace cmx
