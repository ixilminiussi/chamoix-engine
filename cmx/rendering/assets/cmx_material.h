#ifndef CMX_MATERIAL
#define CMX_MATERIAL

// cmx
#include "cmx_assets_manager.h"

// lib
#include "tinyxml2.h"
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_handles.hpp>

// std
#include <memory>

namespace cmx
{

class Material
{
  public:
    Material(std::string vertPath, std::string fragPath);
    virtual ~Material() = default;

    virtual void bind(struct FrameInfo *);
    virtual void editor();

    virtual void initialize();

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

    Material *newInstance()
    {
        return new Material(_id);
    }

    friend void AssetsManager::load(tinyxml2::XMLElement *parentElement);

  protected:
    virtual void createPipelineLayout(std::vector<vk::DescriptorSetLayout>);
    virtual void createPipeline(vk::RenderPass);

    Material(int ID);

    class Actor *parent{nullptr};
    bool _transparent{false};

    const size_t _id;
    static size_t _idProvider;

    static size_t _boundID;

    std::string _vertFilepath;
    std::string _fragFilepath;

    vk::PipelineLayout _pipelineLayout;
    std::unique_ptr<class Pipeline> _pipeline;

    class RenderSystem *_renderSystem{nullptr};

    static Material *_instance;
};

} // namespace cmx

#endif
