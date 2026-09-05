#include "rhi/vulkan/vkrhi.h"


rhi::SamplerHandle VulkanRHI::CreateSampler(const rhi::SamplerDesc& desc) {
    VkSamplerCreateInfo samplerDesc = {};
    samplerDesc.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerDesc.magFilter = toVk(desc.magFilter);
    samplerDesc.minFilter = toVk(desc.minFilter);
    samplerDesc.mipmapMode = toVk(desc.mipFilter);
    samplerDesc.addressModeU = toVk(desc.addressU);
    samplerDesc.addressModeV = toVk(desc.addressV);
    samplerDesc.addressModeW = toVk(desc.addressW);
    samplerDesc.mipLodBias = desc.mipLodBias;
    samplerDesc.anisotropyEnable = desc.anisotropyEnabled;
    samplerDesc.maxAnisotropy = desc.maxAnisotropy;
    samplerDesc.compareEnable = desc.compareEnabled;
    samplerDesc.compareOp = toVk(desc.compareOp);
    samplerDesc.minLod = desc.minLOD;
    samplerDesc.maxLod = desc.maxLOD;
    samplerDesc.borderColor = toVk(desc.borderColor);

    VkSampler sampler;

    if (vkCreateSampler(m_Device, &samplerDesc, nullptr, &sampler) != VK_SUCCESS) {
        printf("[ERROR] Failed to create sampler\n");
        return rhi::SamplerHandle{};
    }

    rhi::SamplerHandle handle;
    handle.id = static_cast<uint64_t>(m_Samplers.size());
    handle.desc = desc;
    m_Samplers.push_back(sampler);
    return handle;
}

void VulkanRHI::DestroySampler(rhi::SamplerHandle handle) {
    if (!m_Device || handle.id == 0xFFFFFFFFFFFFFFFF ||
        handle.id >= m_Samplers.size() ||
        m_Samplers[handle.id] == VK_NULL_HANDLE) {
        return;
    }
    vkDestroySampler(m_Device, m_Samplers[handle.id], nullptr);
    m_Samplers[handle.id] = VK_NULL_HANDLE;
}
