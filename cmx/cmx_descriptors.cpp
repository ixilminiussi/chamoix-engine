#include "cmx_descriptors.h"

// std
#include <cassert>
#include <spdlog/spdlog.h>
#include <stdexcept>
#include <vulkan/vulkan_enums.hpp>
#include <vulkan/vulkan_structs.hpp>

namespace cmx
{

// *************** Descriptor Set Layout Builder *********************

DescriptorSetLayout::Builder &DescriptorSetLayout::Builder::addBinding(uint32_t binding,
                                                                       vk::DescriptorType descriptorType,
                                                                       vk::ShaderStageFlags stageFlags, uint32_t count)
{
    assert(_bindings.count(binding) == 0 && "Binding already in use");
    vk::DescriptorSetLayoutBinding layoutBinding{};
    layoutBinding.binding = binding;
    layoutBinding.descriptorType = descriptorType;
    layoutBinding.descriptorCount = count;
    layoutBinding.stageFlags = stageFlags;
    _bindings[binding] = layoutBinding;
    return *this;
}

std::unique_ptr<DescriptorSetLayout> DescriptorSetLayout::Builder::build() const
{
    return std::make_unique<DescriptorSetLayout>(_device, _bindings);
}

// *************** Descriptor Set Layout *********************

DescriptorSetLayout::DescriptorSetLayout(Device &device,
                                         std::unordered_map<uint32_t, vk::DescriptorSetLayoutBinding> bindings)
    : _device{device}, _bindings{bindings}
{
    std::vector<vk::DescriptorSetLayoutBinding> setLayoutBindings{};
    for (auto kv : bindings)
    {
        setLayoutBindings.push_back(kv.second);
    }

    vk::DescriptorSetLayoutCreateInfo descriptorSetLayoutInfo{};
    descriptorSetLayoutInfo.sType = vk::StructureType::eDescriptorSetLayoutCreateInfo;
    descriptorSetLayoutInfo.bindingCount = static_cast<uint32_t>(setLayoutBindings.size());
    descriptorSetLayoutInfo.pBindings = setLayoutBindings.data();

    if (_device.device().createDescriptorSetLayout(&descriptorSetLayoutInfo, nullptr, &_descriptorSetLayout) !=
        vk::Result::eSuccess)
    {
        throw std::runtime_error("failed to create descriptor set layout!");
    }
}

DescriptorSetLayout::~DescriptorSetLayout()
{
    _device.device().destroyDescriptorSetLayout(_descriptorSetLayout, nullptr);
}

// *************** Descriptor Pool Builder *********************

DescriptorPool::Builder &DescriptorPool::Builder::addPoolSize(vk::DescriptorType descriptorType, uint32_t count)
{
    _poolSizes.push_back({descriptorType, count});
    return *this;
}

DescriptorPool::Builder &DescriptorPool::Builder::setPoolFlags(vk::DescriptorPoolCreateFlags flags)
{
    _poolFlags = flags;
    return *this;
}
DescriptorPool::Builder &DescriptorPool::Builder::setMaxSets(uint32_t count)
{
    _maxSets = count;
    return *this;
}

std::unique_ptr<DescriptorPool> DescriptorPool::Builder::build() const
{
    return std::make_unique<DescriptorPool>(_device, _maxSets, _poolFlags, _poolSizes);
}

// *************** Descriptor Pool *********************

DescriptorPool::DescriptorPool(Device &device, uint32_t maxSets, vk::DescriptorPoolCreateFlags poolFlags,
                               const std::vector<vk::DescriptorPoolSize> &poolSizes)
    : _device{device}
{
    vk::DescriptorPoolCreateInfo descriptorPoolInfo{};
    descriptorPoolInfo.sType = vk::StructureType::eDescriptorPoolCreateInfo;
    descriptorPoolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    descriptorPoolInfo.pPoolSizes = poolSizes.data();
    descriptorPoolInfo.maxSets = maxSets;
    descriptorPoolInfo.flags = poolFlags;

    if (_device.device().createDescriptorPool(&descriptorPoolInfo, nullptr, &_descriptorPool) != vk::Result::eSuccess)
    {
        throw std::runtime_error("failed to create descriptor pool!");
    }
}

DescriptorPool::~DescriptorPool()
{
    if (!_freed)
    {
        spdlog::error("DescriptorPool: forgot to free before deletion");
    }
}

void DescriptorPool::free()
{
    _device.device().destroyDescriptorPool(_descriptorPool, nullptr);
    _freed = true;
}

bool DescriptorPool::allocateDescriptor(const vk::DescriptorSetLayout descriptorSetLayout,
                                        vk::DescriptorSet &descriptor) const
{
    vk::DescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = vk::StructureType::eDescriptorSetAllocateInfo;
    allocInfo.descriptorPool = _descriptorPool;
    allocInfo.pSetLayouts = &descriptorSetLayout;
    allocInfo.descriptorSetCount = 1;

    // Might want to create a "DescriptorPoolManager" class that handles this case, and builds
    // a new pool whenever an old pool fills up. But this is beyond our current scope
    if (_device.device().allocateDescriptorSets(&allocInfo, &descriptor) != vk::Result::eSuccess)
    {
        return false;
    }
    return true;
}

void DescriptorPool::freeDescriptors(std::vector<vk::DescriptorSet> &descriptors) const
{
    _device.device().freeDescriptorSets(_descriptorPool, static_cast<uint32_t>(descriptors.size()), descriptors.data());
}

void DescriptorPool::resetPool()
{
    _device.device().resetDescriptorPool(_descriptorPool, vk::DescriptorPoolResetFlagBits{});
}

// *************** Descriptor Writer *********************

DescriptorWriter::DescriptorWriter(DescriptorSetLayout &setLayout, DescriptorPool &pool)
    : _setLayout{setLayout}, _pool{pool}
{
}

DescriptorWriter::~DescriptorWriter()
{
}

DescriptorWriter &DescriptorWriter::writeBuffer(uint32_t binding, vk::DescriptorBufferInfo *bufferInfo)
{
    assert(_setLayout._bindings.count(binding) == 1 && "Layout does not contain specified binding");

    auto &bindingDescription = _setLayout._bindings[binding];

    assert(bindingDescription.descriptorCount == 1 && "Binding single descriptor info, but binding expects multiple");

    vk::WriteDescriptorSet write{};
    write.sType = vk::StructureType::eWriteDescriptorSet;
    write.descriptorType = bindingDescription.descriptorType;
    write.dstBinding = binding;
    write.pBufferInfo = bufferInfo;
    write.descriptorCount = 1;

    _writes.push_back(write);
    return *this;
}

DescriptorWriter &DescriptorWriter::writeImage(uint32_t binding, vk::DescriptorImageInfo *imageInfo)
{
    assert(_setLayout._bindings.count(binding) == 1 && "Layout does not contain specified binding");

    auto &bindingDescription = _setLayout._bindings[binding];

    assert(bindingDescription.descriptorCount == 1 && "Binding single descriptor info, but binding expects multiple");

    vk::WriteDescriptorSet write{};
    write.sType = vk::StructureType::eWriteDescriptorSet;
    write.descriptorType = bindingDescription.descriptorType;
    write.dstBinding = binding;
    write.pImageInfo = imageInfo;
    write.descriptorCount = 1;

    _writes.push_back(write);
    return *this;
}

bool DescriptorWriter::build(vk::DescriptorSet &set)
{
    bool success = _pool.allocateDescriptor(_setLayout.getDescriptorSetLayout(), set);
    if (!success)
    {
        return false;
    }
    overwrite(set);
    return true;
}

void DescriptorWriter::overwrite(vk::DescriptorSet &set)
{
    for (auto &write : _writes)
    {
        write.dstSet = set;
    }
    _pool._device.device().updateDescriptorSets(_writes.size(), _writes.data(), 0, nullptr);
}

} // namespace cmx
