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
    Material(std::string vertPath, std::string fragPath, bool modelBased = true);
    virtual ~Material() = default;

    virtual void bind(struct FrameInfo *, const class Drawable *) = 0;
    virtual void editor();
    virtual tinyxml2::XMLElement &save(tinyxml2::XMLDocument &doc, tinyxml2::XMLElement *parentElement) const;
    virtual void load(tinyxml2::XMLElement *materialElement);

    std::string getType() const;

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
    bool needsModel() const
    {
        return _modelBased;
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

    bool _modelBased;

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

#define CONCAT_IMPL(x, y) x##y
#define CONCAT(x, y) CONCAT_IMPL(x, y)

#define UNIQUE_ID CONCAT(__COUNTER__, __LINE__)

#define REGISTER_MATERIAL_INTERNAL(Type, ID)                                                                           \
    namespace cmx                                                                                                      \
    {                                                                                                                  \
    struct CONCAT(MaterialRegistrar_, ID)                                                                              \
    {                                                                                                                  \
        CONCAT(MaterialRegistrar_, ID)()                                                                               \
        {                                                                                                              \
            cmx::Register::getInstance().addMaterial(#Type, []() { return new Type(); });                              \
        }                                                                                                              \
    };                                                                                                                 \
    [[maybe_unused]] inline CONCAT(MaterialRegistrar_, ID) CONCAT(actorRegistrar_, ID){};                              \
    }

#define REGISTER_MATERIAL(Type) REGISTER_MATERIAL_INTERNAL(Type, UNIQUE_ID)

} // namespace cmx

#endif
