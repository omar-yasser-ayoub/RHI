#include "rhi/webgpu/wgpurhi.h"
#include <iostream>

bool WebGPURHI::Initialize()
{
    static constexpr auto kTimedWaitAny = wgpu::InstanceFeatureName::TimedWaitAny;
    wgpu::InstanceDescriptor instanceDesc{};
    instanceDesc.requiredFeatureCount = 1;
    instanceDesc.requiredFeatures = &kTimedWaitAny;

    m_Instance = wgpu::CreateInstance(&instanceDesc);
    if (!m_Instance) {
        std::cerr << "Failed to create WebGPU instance.\n";
        return false;
    }

    wgpu::Adapter adapter;
    wgpu::RequestAdapterOptions adapterOpts{};
    auto adapterFuture = m_Instance.RequestAdapter(
        &adapterOpts,
        wgpu::CallbackMode::WaitAnyOnly,
        [&](wgpu::RequestAdapterStatus status,
            wgpu::Adapter a,
            wgpu::StringView message)
        {
            if (status != wgpu::RequestAdapterStatus::Success) {
                std::cerr << "Failed to request adapter: " << message.data << '\n';
                return;
            }
            adapter = std::move(a);
        });

    m_Instance.WaitAny(adapterFuture, UINT64_MAX);

    if (!adapter) {
        std::cerr << "No adapter acquired.\n";
        return false;
    }
    m_Adapter = std::move(adapter);

    wgpu::Device device;
    auto deviceFuture = m_Adapter.RequestDevice(
        nullptr,
        wgpu::CallbackMode::WaitAnyOnly,
        [&](wgpu::RequestDeviceStatus status,
            wgpu::Device d,
            wgpu::StringView message)
        {
            if (status != wgpu::RequestDeviceStatus::Success) {
                std::cerr << "Failed to request device: " << message.data << '\n';
                return;
            }
            device = std::move(d);
        });

    m_Instance.WaitAny(deviceFuture, UINT64_MAX);

    if (!device) {
        std::cerr << "No device acquired.\n";
        return false;
    }
    m_Device = std::move(device);

    m_Queue = m_Device.GetQueue();

    wgpu::AdapterInfo info{};
    m_Adapter.GetInfo(&info);
    m_Capabilities.deviceName = info.device.data;

    m_Capabilities.SupportsMeshShaders = false;
    m_Capabilities.SupportsRayTracing  = false;

    return true;
}

void WebGPURHI::Shutdown()
{
    m_Queue   = {};
    m_Device  = {};
    m_Adapter = {};
    m_Instance = {};
}

void WebGPURHI::PrintCapabilities()
{
    wgpu::AdapterInfo info{};
    m_Adapter.GetInfo(&info);

    std::cout << "========== WebGPU RHI Capabilities ==========\n";
    std::cout << "Device Name : " << std::string(info.device.data, info.device.length) << '\n';
    std::cout << "Vendor      : " << std::string(info.vendor.data, info.vendor.length) << '\n';
    std::cout << "Architecture: " << std::string(info.architecture.data, info.architecture.length) << '\n';
    std::cout << "Driver      : " << std::string(info.description.data, info.description.length) << '\n';
    std::cout << "Backend            : ";

    switch (info.backendType)
    {
    case wgpu::BackendType::D3D11:    std::cout << "D3D11";    break;
    case wgpu::BackendType::D3D12:    std::cout << "D3D12";    break;
    case wgpu::BackendType::Metal:    std::cout << "Metal";    break;
    case wgpu::BackendType::OpenGL:   std::cout << "OpenGL";   break;
    case wgpu::BackendType::OpenGLES: std::cout << "OpenGLES"; break;
    case wgpu::BackendType::Vulkan:   std::cout << "Vulkan";   break;
    case wgpu::BackendType::Null:     std::cout << "Null";     break;
    case wgpu::BackendType::WebGPU:   std::cout << "Browser";  break;
    default:                          std::cout << "Unknown";  break;
    }

    std::cout << '\n';
    std::cout << "Mesh Shaders       : "
              << (m_Capabilities.SupportsMeshShaders ? "Supported" : "Not Exposed") << '\n';
    std::cout << "Ray Tracing        : "
              << (m_Capabilities.SupportsRayTracing  ? "Supported" : "Not Exposed") << '\n';
    std::cout << "=============================================\n";
}
