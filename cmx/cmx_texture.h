#ifndef CMX_TEXTURE
#define CMX_TEXTURE

// lib
#include <stb_image.h>
#include <tinyxml2.h>
#include <vulkan/vulkan.hpp>

// std
#include <string>

namespace cmx
{

class Texture
{
  public:
    struct Builder
    {
        int width;
        int height;
        uint32_t mipLevels;
        vk::Format format{vk::Format::eR8G8B8A8Unorm};
        vk::DeviceSize imageSize;
        stbi_uc *image;
        std::vector<std::string> filepaths;

        void loadTexture(const std::string &filepath);
        void loadTexture3D(const std::vector<std::string> &filepaths);
    };

    Texture(class Device *, const Texture::Builder &, const std::string &name);
    ~Texture();

    Texture(const Texture &) = delete;
    Texture &operator=(const Texture &) = delete;

    void free();

    tinyxml2::XMLElement &save(tinyxml2::XMLDocument &doc, tinyxml2::XMLElement *parentElement);

    void bind(vk::CommandBuffer, vk::PipelineLayout);
    static void bind(vk::CommandBuffer, vk::PipelineLayout, std::vector<Texture *> textures);

    static Texture *createTextureFromFile(class Device *, const std::string &filepath, const std::string &name);

    const std::string name;

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

    bool _freed{false};
};

} // namespace cmx

#endif
