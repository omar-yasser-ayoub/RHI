#include "rhi/dx12/dx12rhi.h"
#include <combaseapi.h>
#include <d3d12.h>
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

    //create sampler descriptor heap
    D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
    heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
    heapDesc.NumDescriptors = 2048;
    heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    m_Device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&m_SamplerHeap));
    m_SamplerDescriptorSize = m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);

    //create render target view descriptor heap
    heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    heapDesc.NumDescriptors = 2048;
    heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    m_Device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&m_RtvHeap));
    m_RtvDescriptorSize = m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

    //create depth stencil view descriptor heap
    heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
    heapDesc.NumDescriptors = 2048;
    heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    m_Device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&m_DsvHeap));
    m_DsvDescriptorSize = m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

    //create cbv/srv/uav descriptor heap
    heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    heapDesc.NumDescriptors = 2048;
    heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    m_Device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&m_CbvSrvUavHeap));
    m_CbvSrvUavDescriptorSize = m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

    return true;
}

void DX12RHI::Shutdown() {
    // ComPtr handles lifecycles, no need to manually clean up
}
