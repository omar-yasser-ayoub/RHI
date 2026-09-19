#include "rhi/webgpu/wgpurhi.h"
#include <iostream>
#include <webgpu/webgpu_cpp.h>

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
    for (auto& buffer : m_Buffers) {
        if (buffer) {
            buffer.Unmap();
            buffer.Destroy();
            buffer = {};
        }
    }
    for (auto& sampler : m_Samplers) {
        sampler = {};
    }

    m_Buffers.clear();
    m_Samplers.clear();
    m_Queue = {};
    m_Device = {};
    m_Adapter = {};
    m_Instance = {};
}

rhi::SwapchainHandle WebGPURHI::CreateSwapchain(const rhi::SwapchainDesc& desc)
{
    return {};
}

void WebGPURHI::DestroySwapchain(rhi::SwapchainHandle handle)
{
}

rhi::AcquireResult WebGPURHI::AcquireNextImage(rhi::SwapchainHandle handle)
{
    return {};
}

void WebGPURHI::Present(rhi::SwapchainHandle handle)
{
}
