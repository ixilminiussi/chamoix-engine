#pragma once

#include "cmx_device.h"

// std
#include <memory>
#include <unordered_map>
#include <vector>

namespace cmx
{

class CmxDescriptorSetLayout
{
  public:
    class Builder
    {
      public:
        Builder(CmxDevice &cmxDevice) : cmxDevice{cmxDevice}
        {
        }

        Builder &addBinding(uint32_t binding, VkDescriptorType descriptorType, VkShaderStageFlags stageFlags,
                            uint32_t count = 1);
        std::unique_ptr<CmxDescriptorSetLayout> build() const;

      private:
        CmxDevice &cmxDevice;
        std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings{};
    };

    CmxDescriptorSetLayout(CmxDevice &cmxDevice, std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings);
    ~CmxDescriptorSetLayout();
    CmxDescriptorSetLayout(const CmxDescriptorSetLayout &) = delete;
    CmxDescriptorSetLayout &operator=(const CmxDescriptorSetLayout &) = delete;

    VkDescriptorSetLayout getDescriptorSetLayout() const
    {
        return descriptorSetLayout;
    }

  private:
    CmxDevice &cmxDevice;
    VkDescriptorSetLayout descriptorSetLayout;
    std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings;

    friend class CmxDescriptorWriter;
};

class CmxDescriptorPool
{
  public:
    class Builder
    {
      public:
        Builder(CmxDevice &cmxDevice) : cmxDevice{cmxDevice}
        {
        }

        Builder &addPoolSize(VkDescriptorType descriptorType, uint32_t count);
        Builder &setPoolFlags(VkDescriptorPoolCreateFlags flags);
        Builder &setMaxSets(uint32_t count);
        std::unique_ptr<CmxDescriptorPool> build() const;

      private:
        CmxDevice &cmxDevice;
        std::vector<VkDescriptorPoolSize> poolSizes{};
        uint32_t maxSets = 1000;
        VkDescriptorPoolCreateFlags poolFlags = 0;
    };

    CmxDescriptorPool(CmxDevice &cmxDevice, uint32_t maxSets, VkDescriptorPoolCreateFlags poolFlags,
                      const std::vector<VkDescriptorPoolSize> &poolSizes);
    ~CmxDescriptorPool();
    CmxDescriptorPool(const CmxDescriptorPool &) = delete;
    CmxDescriptorPool &operator=(const CmxDescriptorPool &) = delete;

    bool allocateDescriptor(const VkDescriptorSetLayout descriptorSetLayout, VkDescriptorSet &descriptor) const;

    void freeDescriptors(std::vector<VkDescriptorSet> &descriptors) const;

    void resetPool();

    VkDescriptorPool getDescriptorPool()
    {
        return descriptorPool;
    }

  private:
    CmxDevice &cmxDevice;
    VkDescriptorPool descriptorPool;

    friend class CmxDescriptorWriter;
};

class CmxDescriptorWriter
{
  public:
    CmxDescriptorWriter(CmxDescriptorSetLayout &setLayout, CmxDescriptorPool &pool);

    CmxDescriptorWriter &writeBuffer(uint32_t binding, VkDescriptorBufferInfo *bufferInfo);
    CmxDescriptorWriter &writeImage(uint32_t binding, VkDescriptorImageInfo *imageInfo);

    bool build(VkDescriptorSet &set);
    void overwrite(VkDescriptorSet &set);

  private:
    CmxDescriptorSetLayout &setLayout;
    CmxDescriptorPool &pool;
    std::vector<VkWriteDescriptorSet> writes;
};

} // namespace cmx
