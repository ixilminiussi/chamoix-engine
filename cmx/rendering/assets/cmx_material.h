#ifndef CMX_MATERIAL
#define CMX_MATERIAL

// cmx
#include "cmx_assets_manager.h"

// lib
#include <SPIRV-Reflect/spirv_reflect.h>
#include <tinyxml2.h>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_enums.hpp>
#include <vulkan/vulkan_handles.hpp>

// std
#include <memory>

namespace cmx
{

struct BindingInfo
{
    uint32_t set;
    uint32_t binding;
    SpvReflectDescriptorType type;
    vk::ShaderStageFlagBits shaderStage;
};

class Material
{
  public:
    Material(std::string vertPath, std::string fragPath);
    virtual ~Material() = default;

    virtual void bind(struct FrameInfo *, const class Drawable *) = 0;
    virtual void editor();

    virtual void initialize();
    virtual void free();

    bool isTransparent() const
    {
        return _transparent;
    }
    size_t getID() const
    {
        return _id;
    }
    vk::PipelineLayout getPipelineLayout() const
    {
        return _pipelineLayout;
    }

    static void resetBoundID();

    const std::vector<BindingInfo> &getBindings() const
    {
        return _bindings;
    }
    size_t getTotalSamplers() const;

    void loadBindings();
    // virtual Material *newInstance() = 0;

    friend void AssetsManager::load(tinyxml2::XMLElement *parentElement);

    std::string name;

  protected:
    Material(int ID);

    virtual void createPipelineLayout(std::vector<vk::DescriptorSetLayout>) = 0;
    virtual void createPipeline(vk::RenderPass) = 0;

    static std::vector<SpvReflectDescriptorBinding *> loadBindings(const std::string &filename);
    static std::vector<uint32_t> loadSpirvData(const std::string &filename);

    class Actor *parent{nullptr};
    bool _transparent{false};

    const size_t _id;
    static size_t _idProvider;

    static size_t _boundID;

    std::string _vertFilepath;
    std::string _fragFilepath;

    vk::PipelineLayout _pipelineLayout;
    std::unique_ptr<class Pipeline> _pipeline;

    std::vector<BindingInfo> _bindings;

    class RenderSystem *_renderSystem{nullptr};

    static Material *_instance;
};

} // namespace cmx

#endif
