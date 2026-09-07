#include "rhi/dx12/dx12rhi.h"
#include <array>

rhi::SamplerHandle DX12RHI::CreateSampler(const rhi::SamplerDesc& desc) {
    D3D12_SAMPLER_DESC samplerDesc{};
    samplerDesc.Filter = toD3D12(desc.minFilter, desc.magFilter, desc.mipFilter, desc.anisotropyEnabled, desc.compareEnabled);
    samplerDesc.AddressU = toD3D12(desc.addressU);
    samplerDesc.AddressV = toD3D12(desc.addressV);
    samplerDesc.AddressW = toD3D12(desc.addressW);
    samplerDesc.MipLODBias = desc.mipLodBias;
    samplerDesc.MaxAnisotropy = desc.maxAnisotropy;
    samplerDesc.ComparisonFunc = toD3D12(desc.compareOp);
    auto borderColor = toD3D12(desc.borderColor);
    memcpy(samplerDesc.BorderColor, borderColor.data(), sizeof(samplerDesc.BorderColor));
    samplerDesc.MinLOD = desc.minLOD;
    samplerDesc.MaxLOD = desc.maxLOD;

    SamplerViewData viewData{};

    auto handles = AllocateDescriptor(
        m_SamplerHeap,
        m_SamplerHeapOffset,
        m_SamplerDescriptorSize);
    if (!handles.valid) {
        return rhi::SamplerHandle{};
    }

    viewData.cpu = handles.cpu;
    viewData.gpu = handles.gpu;
    viewData.hasSampler = true;

    m_Device->CreateSampler(&samplerDesc, handles.cpu);

    const auto id = static_cast<std::uint64_t>(m_SamplerViews.size());
    m_SamplerViews.push_back(viewData);

    rhi::SamplerHandle result;
    result.id = id;
    result.desc = desc;
    return result;
}

void DX12RHI::DestroySampler(rhi::SamplerHandle handle) {
    return;
}
