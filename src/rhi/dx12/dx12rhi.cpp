#include "rhi/dx12/dx12rhi.h"
#include "rhi/IRHI.h"
#include <combaseapi.h>
#include <iostream>

bool DX12RHI::Initialize() {

    HRESULT result = CreateDXGIFactory2(0, IID_PPV_ARGS(&m_Factory));
    if (FAILED(result)) {
        std::cerr << "Failed to create DXGI factory: " << result << std::endl;
        return false;
    }

    result = m_Factory->EnumAdapterByGpuPreference(
        0,
        DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE,
        IID_PPV_ARGS(&m_Adapter)
    );
    if (FAILED(result)) {
        std::cerr << "Failed to enumerate adapter by GPU preference: " << result << std::endl;
        return false;
    }
    DXGI_ADAPTER_DESC1 desc;
    result = m_Adapter->GetDesc1(&desc);
    if (FAILED(result)) {
        std::cerr << "Failed to get adapter description: " << result << std::endl;
        return false;
    }

    std::wstring wName(desc.Description);
    m_Capabilities.deviceName = std::string(wName.begin(), wName.end());

    result = D3D12CreateDevice(m_Adapter.Get(), D3D_FEATURE_LEVEL_12_1, IID_PPV_ARGS(&m_Device));
    if (FAILED(result)) {
        std::cerr << "Failed to create D3D12 device: " << result << std::endl;
        return false;
    }

    D3D12_FEATURE_DATA_D3D12_OPTIONS7 options7 = {};
    result = m_Device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS7, &options7, sizeof(options7));
    if (SUCCEEDED(result)) {
            // In DX12, mesh shaders are supported if the tier is Tier 1 or higher
            m_Capabilities.SupportsMeshShaders = (options7.MeshShaderTier != D3D12_MESH_SHADER_TIER_NOT_SUPPORTED);
    }

    D3D12_FEATURE_DATA_D3D12_OPTIONS5 options5 = {};
    result = m_Device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS5, &options5, sizeof(options5));
    if (SUCCEEDED(result)) {
        // Ray tracing is supported if the tier is 1.0 or 1.1
        m_Capabilities.SupportsRayTracing = (options5.RaytracingTier != D3D12_RAYTRACING_TIER_NOT_SUPPORTED);
    }

    return true;
}

void DX12RHI::Shutdown() {
    // ComPtr handles lifecycles, no need to manually clean up
}

rhi::SamplerHandle CreateSampler(const rhi::SamplerDesc& desc) {

}

rhi::TextureHandle CreateTexture(const rhi::TextureDesc& desc) {

}

rhi::BufferHandle CreateBuffer(const rhi::BufferDesc& desc, const void* initialData = nullptr) {

}

void DestroySampler(rhi::SamplerHandle handle) {

}

void DestroyTexture(rhi::TextureHandle handle) {

}

void DestroyBuffer(rhi::BufferHandle handle) {

}

void WriteBuffer(rhi::BufferHandle handle, const void* data, std::size_t size, std::size_t offset = 0) {

}

void WriteTexture(rhi::TextureHandle handle, const rhi::TextureWriteDesc& desc, const void* data) {

}
