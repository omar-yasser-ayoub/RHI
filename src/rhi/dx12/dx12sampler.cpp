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

    D3D12_CPU_DESCRIPTOR_HANDLE handle = m_SamplerHeap->GetCPUDescriptorHandleForHeapStart();
    handle.ptr += m_SamplerHeapOffset * m_SamplerDescriptorSize;

    m_Device->CreateSampler(&samplerDesc, handle);

    rhi::SamplerHandle result;
    result.id = m_SamplerHeapOffset;
    result.desc = desc;

    ++m_SamplerHeapOffset;
    return result;
}

void DX12RHI::DestroySampler(rhi::SamplerHandle handle) {
    return;
}
