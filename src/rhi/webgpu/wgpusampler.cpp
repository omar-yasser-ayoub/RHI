#include "rhi/webgpu/wgpurhi.h"

rhi::SamplerHandle WebGPURHI::CreateSampler(const rhi::SamplerDesc& desc) {
    rhi::SamplerHandle handle;

    //check all passed parameters in SamplerDesc, make a GPUSamplerDescriptor, create the sampler, and log if any features are not supported.

    wgpu::SamplerDescriptor samplerDesc{};
    samplerDesc.minFilter = toWebGPU(desc.minFilter);
    samplerDesc.magFilter = toWebGPU(desc.magFilter);
    samplerDesc.mipmapFilter = toWebGPU(desc.mipFilter);
    samplerDesc.addressModeU = toWebGPU(desc.addressU);
    samplerDesc.addressModeV = toWebGPU(desc.addressV);
    samplerDesc.addressModeW = toWebGPU(desc.addressW);
    samplerDesc.lodMinClamp = desc.minLOD;
    samplerDesc.lodMaxClamp = desc.maxLOD;
    samplerDesc.maxAnisotropy = desc.maxAnisotropy;
    samplerDesc.compare = toWebGPU(desc.compareOp);

    if (desc.borderColor != rhi::BorderColor::TransparentBlack) {
        printf("[RHI] Warning: BorderColors are unsupported in WebGPU.\n");
    }
    if (desc.mipLodBias != 0.0f) {
        printf("[RHI] Warning: mipLodBias is unsupported in WebGPU.\n");
    }
    if (desc.anisotropyEnabled) {
        printf("[RHI] Warning: anisotropyEnabled parameter is unsupported in WebGPU. Writing to maxAnisotropy enables anisotropy implicitly. \n");
    }
    if (desc.compareEnabled) {
        printf("[RHI] Warning: compareEnabled parameter is unsupported in WebGPU. Writing to compareOp enables comparison implicitly. \n");
    }

    wgpu::Sampler sampler = m_Device.CreateSampler(&samplerDesc);
    if (!sampler) {
        printf("[RHI] Failed to create WebGPU sampler\n");
        return handle;
    }
    handle.id = static_cast<uint64_t>(m_Samplers.size());
    handle.desc = desc;
    m_Samplers.push_back(sampler);

    return handle;
}

void WebGPURHI::DestroySampler(rhi::SamplerHandle handle) {
    if (handle.id == 0xFFFFFFFFFFFFFFFF || handle.id >= m_Samplers.size()) {
        return;
    }
    m_Samplers[handle.id] = {};
}
