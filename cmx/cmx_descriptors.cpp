#include "cmx_descriptors.h"

// std
#include <cassert>
#include <stdexcept>

namespace cmx
{

// *************** Descriptor Set Layout Builder *********************

CmxDescriptorSetLayout::Builder &CmxDescriptorSetLayout::Builder::addBinding(uint32_t binding,
                                                                             VkDescriptorType descriptorType,
                                                                             VkShaderStageFlags stageFlags,
                                                                             uint32_t count)
{
    assert(_bindings.count(binding) == 0 && "Binding already in use");
    VkDescriptorSetLayoutBinding layoutBinding{};
    layoutBinding.binding = binding;
    layoutBinding.descriptorType = descriptorType;
    layoutBinding.descriptorCount = count;
    layoutBinding.stageFlags = stageFlags;
    _bindings[binding] = layoutBinding;
    return *this;
}

std::unique_ptr<CmxDescriptorSetLayout> CmxDescriptorSetLayout::Builder::build() const
{
    return std::make_unique<CmxDescriptorSetLayout>(_cmxDevice, _bindings);
}

// *************** Descriptor Set Layout *********************

CmxDescriptorSetLayout::CmxDescriptorSetLayout(CmxDevice &cmxDevice,
                                               std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings)
    : cmxDevice{cmxDevice}, bindings{bindings}
{
    std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings{};
    for (auto kv : bindings)
    {
        setLayoutBindings.push_back(kv.second);
    }

    VkDescriptorSetLayoutCreateInfo descriptorSetLayoutInfo{};
    descriptorSetLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    descriptorSetLayoutInfo.bindingCount = static_cast<uint32_t>(setLayoutBindings.size());
    descriptorSetLayoutInfo.pBindings = setLayoutBindings.data();

    if (vkCreateDescriptorSetLayout(cmxDevice.device(), &descriptorSetLayoutInfo, nullptr, &descriptorSetLayout) !=
        VK_SUCCESS)
    {
        throw std::runtime_error("failed to create descriptor set layout!");
    }
}

CmxDescriptorSetLayout::~CmxDescriptorSetLayout()
{
    vkDestroyDescriptorSetLayout(cmxDevice.device(), descriptorSetLayout, nullptr);
}

// *************** Descriptor Pool Builder *********************

CmxDescriptorPool::Builder &CmxDescriptorPool::Builder::addPoolSize(VkDescriptorType descriptorType, uint32_t count)
{
    _poolSizes.push_back({descriptorType, count});
    return *this;
}

CmxDescriptorPool::Builder &CmxDescriptorPool::Builder::setPoolFlags(VkDescriptorPoolCreateFlags flags)
{
    _poolFlags = flags;
    return *this;
}
CmxDescriptorPool::Builder &CmxDescriptorPool::Builder::setMaxSets(uint32_t count)
{
    _maxSets = count;
    return *this;
}

std::unique_ptr<CmxDescriptorPool> CmxDescriptorPool::Builder::build() const
{
    return std::make_unique<CmxDescriptorPool>(_cmxDevice, _maxSets, _poolFlags, _poolSizes);
}

// *************** Descriptor Pool *********************

CmxDescriptorPool::CmxDescriptorPool(CmxDevice &cmxDevice, uint32_t maxSets, VkDescriptorPoolCreateFlags poolFlags,
                                     const std::vector<VkDescriptorPoolSize> &poolSizes)
    : _cmxDevice{cmxDevice}
{
    VkDescriptorPoolCreateInfo descriptorPoolInfo{};
    descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    descriptorPoolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    descriptorPoolInfo.pPoolSizes = poolSizes.data();
    descriptorPoolInfo.maxSets = maxSets;
    descriptorPoolInfo.flags = poolFlags;

    if (vkCreateDescriptorPool(cmxDevice.device(), &descriptorPoolInfo, nullptr, &_descriptorPool) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create descriptor pool!");
    }
}

CmxDescriptorPool::~CmxDescriptorPool()
{
    vkDestroyDescriptorPool(_cmxDevice.device(), _descriptorPool, nullptr);
}

bool CmxDescriptorPool::allocateDescriptor(const VkDescriptorSetLayout descriptorSetLayout,
                                           VkDescriptorSet &descriptor) const
{
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = _descriptorPool;
    allocInfo.pSetLayouts = &descriptorSetLayout;
    allocInfo.descriptorSetCount = 1;

    // Might want to create a "DescriptorPoolManager" class that handles this case, and builds
    // a new pool whenever an old pool fills up. But this is beyond our current scope
    if (vkAllocateDescriptorSets(_cmxDevice.device(), &allocInfo, &descriptor) != VK_SUCCESS)
    {
        return false;
    }
    return true;
}

void CmxDescriptorPool::freeDescriptors(std::vector<VkDescriptorSet> &descriptors) const
{
    vkFreeDescriptorSets(_cmxDevice.device(), _descriptorPool, static_cast<uint32_t>(descriptors.size()),
                         descriptors.data());
}

void CmxDescriptorPool::resetPool()
{
    vkResetDescriptorPool(_cmxDevice.device(), _descriptorPool, 0);
}

// *************** Descriptor Writer *********************

CmxDescriptorWriter::CmxDescriptorWriter(CmxDescriptorSetLayout &setLayout, CmxDescriptorPool &pool)
    : _setLayout{setLayout}, _pool{pool}
{
}

CmxDescriptorWriter &CmxDescriptorWriter::writeBuffer(uint32_t binding, VkDescriptorBufferInfo *bufferInfo)
{
    assert(_setLayout.bindings.count(binding) == 1 && "Layout does not contain specified binding");

    auto &bindingDescription = _setLayout.bindings[binding];

    assert(bindingDescription.descriptorCount == 1 && "Binding single descriptor info, but binding expects multiple");

    VkWriteDescriptorSet write{};
    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.descriptorType = bindingDescription.descriptorType;
    write.dstBinding = binding;
    write.pBufferInfo = bufferInfo;
    write.descriptorCount = 1;

    _writes.push_back(write);
    return *this;
}

CmxDescriptorWriter &CmxDescriptorWriter::writeImage(uint32_t binding, VkDescriptorImageInfo *imageInfo)
{
    assert(_setLayout.bindings.count(binding) == 1 && "Layout does not contain specified binding");

    auto &bindingDescription = _setLayout.bindings[binding];

    assert(bindingDescription.descriptorCount == 1 && "Binding single descriptor info, but binding expects multiple");

    VkWriteDescriptorSet write{};
    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.descriptorType = bindingDescription.descriptorType;
    write.dstBinding = binding;
    write.pImageInfo = imageInfo;
    write.descriptorCount = 1;

    _writes.push_back(write);
    return *this;
}

bool CmxDescriptorWriter::build(VkDescriptorSet &set)
{
    bool success = _pool.allocateDescriptor(_setLayout.getDescriptorSetLayout(), set);
    if (!success)
    {
        return false;
    }
    overwrite(set);
    return true;
}

void CmxDescriptorWriter::overwrite(VkDescriptorSet &set)
{
    for (auto &write : _writes)
    {
        write.dstSet = set;
    }
    vkUpdateDescriptorSets(_pool._cmxDevice.device(), _writes.size(), _writes.data(), 0, nullptr);
}

} // namespace cmx
