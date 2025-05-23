#ifndef CMX_TEXTURE
#define CMX_TEXTURE

// lib
#include <stb_image.h>
#include <tinyxml2.h>
#include <vulkan/vulkan.hpp>

// std
#include <string>
#include <vulkan/vulkan_enums.hpp>

namespace cmx
{

class Texture
{
  public:
    struct Builder
    {
        int width;
        int height;
        int depth{1};
        uint32_t mipLevels;
        vk::Format format{vk::Format::eR8G8B8A8Unorm};
        vk::DeviceSize imageSize;
        std::vector<stbi_uc *> images;
        std::vector<std::string> filepaths;

        void loadTexture(const char *filepath);
        void loadTexture3D(const std::vector<std::string> &filepaths);
    };

    Texture(class Device *, const Texture::Builder &, const char *name);
    ~Texture();

    Texture(const Texture &) = delete;
    Texture &operator=(const Texture &) = delete;

    void free();

    tinyxml2::XMLElement &save(tinyxml2::XMLDocument &doc, tinyxml2::XMLElement *parentElement);

    void editor();

    void bind(vk::CommandBuffer, vk::PipelineLayout);
    static void bindMany(vk::CommandBuffer, vk::PipelineLayout, std::vector<Texture *> textures);

    static Texture *create2DTextureFromFile(class Device *, const char *filepath, const char *name);
    static Texture *create3DTextureFromFile(class Device *, const std::vector<std::string> &filepaths,
                                            const char *name);
    static void resetBoundID();

    const std::string name;

    vk::ImageType getType()
    {
        return _imageType;
    }

  protected:
    void createImage(class Device *, const Builder &);
    void createImageView(class Device *, const Builder &);
    void createSampler(class Device *);
    void generateMipmaps(class Device *, const Builder &);

    vk::Image _image;
    vk::ImageView _imageView;
    vk::DeviceMemory _imageMemory;

    vk::Sampler _sampler;
    size_t _descriptorSetID;

    std::vector<std::string> _filepaths;
    vk::ImageType _imageType;

    bool _freed{false};

    class RenderSystem *_renderSystem;

    static size_t _boundID;
};

} // namespace cmx

#endif
