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
        vk::DeviceSize imageSize;
        stbi_uc *image;
        std::string filepath;

        void loadTexture(const std::string &filepath);
    };

    Texture(class Device *, const Texture::Builder &, const std::string &name);
    ~Texture();

    Texture(const Texture &) = delete;
    Texture &operator=(const Texture &) = delete;

    void free();

    tinyxml2::XMLElement &save(tinyxml2::XMLDocument &doc, tinyxml2::XMLElement *parentElement);

    void bind(vk::CommandBuffer);
    void draw(vk::CommandBuffer);

    static Texture *createTextureFromFile(class Device *, const std::string &filepath, const std::string &name);

    const std::string name;

  protected:
    void createTextureImage(Device *device, const Builder &builder);

    vk::Image _textureImage;
    vk::ImageView _textureImageView;
    vk::DeviceMemory _textureImageMemory;

    std::string _filepath;

    bool _freed{false};
};

} // namespace cmx

#endif
