#include "rhi/rhi.h"
#include <combaseapi.h>
#include <d3d12.h>
#include <dxgi.h>
#include<dxgi1_6.h>
#include <iostream>
#include <winnt.h>
#include<wrl/client.h>

using Microsoft::WRL::ComPtr;

RHI::RHI() {}

RHI::~RHI() {}

bool RHI::Initialize() {
    ComPtr<IDXGIFactory6> factory;
    HRESULT result = CreateDXGIFactory2(0, IID_PPV_ARGS(&factory));
    if (FAILED(result)) {
        std::cerr << "Failed to create DXGI factory: " << result << std::endl;
        return false;
    }

    ComPtr<IDXGIAdapter1> adapter;
    result = factory->EnumAdapterByGpuPreference(
        0,
        DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE,
        IID_PPV_ARGS(&adapter)
    );
    if (FAILED(result)) {
        std::cerr << "Failed to enumerate adapter by GPU preference: " << result << std::endl;
        return false;
    }
    DXGI_ADAPTER_DESC1 desc;
    result = adapter->GetDesc1(&desc);
    if (FAILED(result)) {
        std::cerr << "Failed to get adapter description: " << result << std::endl;
        return false;
    }

    std::wstring wName(desc.Description);
    m_Capabilities.deviceName = std::string(wName.begin(), wName.end());

    ComPtr<ID3D12Device> device;
    result = D3D12CreateDevice(adapter.Get(), D3D12_FL_11_0, IID_PPV_ARGS(&device));
    if (FAILED(result)) {
        std::cerr << "Failed to create D3D12 device: " << result << std::endl;
        return false;
    }

    D3D12_FEATURE_DATA_D3D12_OPTIONS7 options7 = {};
    result = device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS7, &options7, sizeof(options7));
    if (SUCCEEDED(result)) {
            // In DX12, mesh shaders are supported if the tier is Tier 1 or higher
            m_Capabilities.SupportsMeshShaders = (options7.MeshShaderTier != D3D12_MESH_SHADER_TIER_NOT_SUPPORTED);
    }

    D3D12_FEATURE_DATA_D3D12_OPTIONS5 options5 = {};
    result = device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS5, &options5, sizeof(options5));
    if (SUCCEEDED(result)) {
        // Ray tracing is supported if the tier is 1.0 or 1.1
        m_Capabilities.SupportsRayTracing = (options5.RaytracingTier != D3D12_RAYTRACING_TIER_NOT_SUPPORTED);
    }

    return true;
}

void RHI::Shutdown() {
    // ComPtr handles lifecycles, no need to manually clean up
}

void RHI::PrintCapabilities() {
    std::cout << "========== DirectX 12 RHI Capabilities ==========\n";
    std::cout << "Device Name        : " << m_Capabilities.deviceName << '\n';
    std::cout << "Mesh Shaders       : "
              << (m_Capabilities.SupportsMeshShaders ? "Supported" : "Not Supported") << '\n';
    std::cout << "Ray Tracing        : "
              << (m_Capabilities.SupportsRayTracing ? "Supported" : "Not Supported") << '\n';
    std::cout << "=================================================\n";
}

const RHICapabilities& RHI::GetCapabilities() const {
    return m_Capabilities;
}
