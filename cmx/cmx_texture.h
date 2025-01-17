#ifndef CMX_TEXTURE
#define CMX_TEXTURE

// lib
#include <tinyxml2.h>
#include <vulkan/vulkan_core.h>

// std
#include <memory>
#include <string>

#define STB_IMAGE_IMPLEMENTATION

namespace cmx
{

class CmxTexture
{
  public:
    struct Builder
    {
        std::string filepath;
        uint32_t texWidth, texHeight;
        VkDeviceSize imageSize;
        unsigned char *pixels;

        void loadTexture(const std::string &filepath);
    };

    CmxTexture(class CmxDevice *, const CmxTexture::Builder &, const std::string &name);
    ~CmxTexture();

    CmxTexture(const CmxTexture &) = delete;
    CmxTexture &operator=(const CmxTexture &) = delete;

    tinyxml2::XMLElement &save(tinyxml2::XMLDocument &doc, tinyxml2::XMLElement *parentElement);

    static CmxTexture *createTextureFromFile(class CmxDevice *, const std::string &filepath, const std::string &name);

    void bind(VkCommandBuffer);
    void free();

    const std::string name;

  private:
    std::unique_ptr<class CmxBuffer> _stagingBuffer;
    VkImage textureImage;
    VkDeviceMemory textureImageMemory;

    std::string _filepath;

    bool _freed{false};
};

} // namespace cmx

#endif
