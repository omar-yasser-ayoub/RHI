#include "rhi/dx12/dx12rhi.h"
#include "rhi/irhi.h"
#include <combaseapi.h>
#include <d3d12.h>
#include <iostream>
#include <array>


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

    //create sampler descriptor heap
    D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
    heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
    heapDesc.NumDescriptors = 2048;
    heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    m_Device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&m_SamplerHeap));
    m_SamplerDescriptorSize = m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
    return true;
}

void DX12RHI::Shutdown() {
    // ComPtr handles lifecycles, no need to manually clean up
}

D3D12_FILTER DX12RHI::toD3D12(rhi::Filter min, rhi::Filter mag, rhi::MipmapFilter mip, bool anisotropic, bool compare) {
    uint32_t filter = 0;

    if (anisotropic)                                filter |= 0x54;
    else
        {
            if (min == rhi::Filter::Linear)       filter |= 0x10;
            if (mag == rhi::Filter::Linear)       filter |= 0x04;
        }
    if (mip == rhi::MipmapFilter::Linear)     filter |= 0x01;
    if (compare)                              filter |= 0x80;

    return static_cast<D3D12_FILTER>(filter);
}

D3D12_TEXTURE_ADDRESS_MODE DX12RHI::toD3D12(rhi::AddressMode mode) {
    switch (mode) {
        case rhi::AddressMode::Repeat: return D3D12_TEXTURE_ADDRESS_MODE_WRAP;
        case rhi::AddressMode::MirrorRepeat: return D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
        case rhi::AddressMode::ClampToEdge: return D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
        case rhi::AddressMode::ClampToBorder: return D3D12_TEXTURE_ADDRESS_MODE_BORDER;
        case rhi::AddressMode::MirrorClampToEdge: return D3D12_TEXTURE_ADDRESS_MODE_MIRROR_ONCE;
        default: return D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    }
}

D3D12_COMPARISON_FUNC DX12RHI::toD3D12(rhi::CompareOp mode) {
    switch (mode) {
        case rhi::CompareOp::Never: return D3D12_COMPARISON_FUNC_NEVER;
        case rhi::CompareOp::Less: return D3D12_COMPARISON_FUNC_LESS;
        case rhi::CompareOp::Equal: return D3D12_COMPARISON_FUNC_EQUAL;
        case rhi::CompareOp::LessEqual: return D3D12_COMPARISON_FUNC_LESS_EQUAL;
        case rhi::CompareOp::Greater: return D3D12_COMPARISON_FUNC_GREATER;
        case rhi::CompareOp::NotEqual: return D3D12_COMPARISON_FUNC_NOT_EQUAL;
        case rhi::CompareOp::GreaterEqual: return D3D12_COMPARISON_FUNC_GREATER_EQUAL;
        case rhi::CompareOp::Always: return D3D12_COMPARISON_FUNC_ALWAYS;
        default: return D3D12_COMPARISON_FUNC_NONE;
    }
}

std::array<float, 4> DX12RHI::toD3D12(rhi::BorderColor color) {
    switch (color) {
        case rhi::BorderColor::TransparentBlack: return { 0.0f, 0.0f, 0.0f, 0.0f };
        case rhi::BorderColor::OpaqueBlack: return { 0.0f, 0.0f, 0.0f, 1.0f };
        case rhi::BorderColor::OpaqueWhite: return { 1.0f, 1.0f, 1.0f, 1.0f };
        default: return { 0.0f, 0.0f, 0.0f, 0.0f };
    }
}

D3D12_HEAP_PROPERTIES DX12RHI::toD3D12(rhi::MemoryUsage usage) {
    switch (usage)
    {
        case rhi::MemoryUsage::GpuOnly:  return D3D12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
        case rhi::MemoryUsage::CpuToGpu: return D3D12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
        case rhi::MemoryUsage::GpuToCpu: return D3D12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_READBACK);
        default:                         return D3D12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
    }
}

D3D12_RESOURCE_STATES toD3D12InitialState(rhi::MemoryUsage usage) {
    switch (usage)
    {
        case rhi::MemoryUsage::GpuOnly:  return D3D12_RESOURCE_STATE_COMMON;
        case rhi::MemoryUsage::CpuToGpu: return D3D12_RESOURCE_STATE_GENERIC_READ;
        case rhi::MemoryUsage::GpuToCpu: return D3D12_RESOURCE_STATE_COPY_DEST;
        default:                         return D3D12_RESOURCE_STATE_COMMON;
    }
}

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

    ++m_SamplerHeapOffset;
    return result;
}

rhi::TextureHandle DX12RHI::CreateTexture(const rhi::TextureDesc& desc) {
    return rhi::TextureHandle{};
}

rhi::BufferHandle DX12RHI::CreateBuffer(const rhi::BufferDesc& desc) {
    D3D12_HEAP_PROPERTIES heapProps = toD3D12(desc.memoryUsage);
    D3D12_RESOURCE_STATES initialState = toD3D12InitialState(desc.memoryUsage);

    D3D12_RESOURCE_DESC resourceDesc = {};
    resourceDesc.Dimension          = D3D12_RESOURCE_DIMENSION_BUFFER;
    resourceDesc.Alignment          = 0;
    resourceDesc.Width              = desc.size;
    resourceDesc.Height             = 1;
    resourceDesc.DepthOrArraySize   = 1;
    resourceDesc.MipLevels          = 1;
    resourceDesc.Format             = DXGI_FORMAT_UNKNOWN;
    resourceDesc.SampleDesc.Count   = 1;
    resourceDesc.SampleDesc.Quality = 0;
    resourceDesc.Layout             = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    resourceDesc.Flags              = D3D12_RESOURCE_FLAG_NONE;

    Microsoft::WRL::ComPtr<ID3D12Resource> resource;
        if (FAILED(m_Device->CreateCommittedResource(
            &heapProps,
            D3D12_HEAP_FLAG_NONE,
            &resourceDesc,
            initialState,
            nullptr,
            IID_PPV_ARGS(&resource))))
        {
            printf("[ERROR] Failed to create buffer\n");
            return rhi::BufferHandle{};
        }

    rhi::BufferHandle handle;
    handle.id = static_cast<uint64_t>(m_Buffers.size());
    m_Buffers.push_back(std::move(resource));
    return handle;

    return rhi::BufferHandle{};
}

void DX12RHI::DestroySampler(rhi::SamplerHandle handle) {
    return;
}

void DX12RHI::DestroyTexture(rhi::TextureHandle handle) {
    return;
}

void DX12RHI::DestroyBuffer(rhi::BufferHandle handle)
{
    if (!m_Device || handle.id == 0xFFFFFFFFFFFFFFFF ||
        handle.id >= m_Buffers.size()) {
        return;
    }
    m_Buffers[handle.id].Reset();
}

void* DX12RHI::MapBuffer(rhi::BufferHandle handle) {
    if (!m_Device || handle.id == 0xFFFFFFFFFFFFFFFF ||
        handle.id >= m_Buffers.size()) {
        return nullptr;
    }

        void* ptr = nullptr;
        if (FAILED(m_Buffers[handle.id]->Map(0, nullptr, &ptr)))
        {
            printf("[ERROR] Failed to map buffer\n");
            return nullptr;
        }

        return ptr;
}

void DX12RHI::UnmapBuffer(rhi::BufferHandle handle) {
    if (!m_Device || handle.id == 0xFFFFFFFFFFFFFFFF ||
        handle.id >= m_Buffers.size()) {
        return;
    }
    m_Buffers[handle.id]->Unmap(0, nullptr);
}

void DX12RHI::WriteBuffer(rhi::BufferHandle handle, const void* data, std::size_t size, std::size_t offset) {
    return;
}

void DX12RHI::WriteTexture(rhi::TextureHandle handle, const rhi::TextureWriteDesc& desc, const void* data) {
    return;
}
