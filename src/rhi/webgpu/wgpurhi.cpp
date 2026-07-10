#include "rhi/webgpu/wgpurhi.h"
#include "rhi/irhi.h"
#include <cstdio>
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
    m_Queue   = {};
    m_Device  = {};
    m_Adapter = {};
    m_Instance = {};
}

wgpu::FilterMode WebGPURHI::toWebGPU(rhi::Filter f) {
    switch (f) {
        case rhi::Filter::Nearest: return wgpu::FilterMode::Nearest;
        case rhi::Filter::Linear:  return wgpu::FilterMode::Linear;
        default: return wgpu::FilterMode::Nearest;
    }
}

wgpu::MipmapFilterMode WebGPURHI::toWebGPU(rhi::MipmapFilter f) {
    switch (f) {
        case rhi::MipmapFilter::Nearest: return wgpu::MipmapFilterMode::Nearest;
        case rhi::MipmapFilter::Linear:  return wgpu::MipmapFilterMode::Linear;
        default: return wgpu::MipmapFilterMode::Nearest;
    }
}

wgpu::AddressMode WebGPURHI::toWebGPU(rhi::AddressMode f) {
    switch (f) {
        case rhi::AddressMode::Repeat: return wgpu::AddressMode::Repeat;
        case rhi::AddressMode::MirrorRepeat: return wgpu::AddressMode::MirrorRepeat;
        case rhi::AddressMode::ClampToEdge: return wgpu::AddressMode::ClampToEdge;
        case rhi::AddressMode::ClampToBorder:
            printf("[RHI] Warning: ClampToBorder unsupported in WebGPU, falling back to ClampToEdge\n");
            return wgpu::AddressMode::ClampToEdge;
        case rhi::AddressMode::MirrorClampToEdge:
            printf("[RHI] Warning: MirrorClampToEdge unsupported in WebGPU, falling back to MirrorRepeat\n");
            return wgpu::AddressMode::MirrorRepeat;
        default: return wgpu::AddressMode::Repeat;
    }
}

wgpu::CompareFunction WebGPURHI::toWebGPU(rhi::CompareOp op) {
    switch (op) {
        case rhi::CompareOp::Never: return wgpu::CompareFunction::Never;
        case rhi::CompareOp::Less: return wgpu::CompareFunction::Less;
        case rhi::CompareOp::Equal: return wgpu::CompareFunction::Equal;
        case rhi::CompareOp::LessEqual: return wgpu::CompareFunction::LessEqual;
        case rhi::CompareOp::Greater: return wgpu::CompareFunction::Greater;
        case rhi::CompareOp::NotEqual: return wgpu::CompareFunction::NotEqual;
        case rhi::CompareOp::GreaterEqual: return wgpu::CompareFunction::GreaterEqual;
        case rhi::CompareOp::Always: return wgpu::CompareFunction::Always;
        default: return wgpu::CompareFunction::Undefined;
    }
}

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
    handle.id = static_cast<uint64_t>(m_Samplers.size());
    m_Samplers.push_back(sampler);

    return handle;
}

rhi::TextureHandle WebGPURHI::CreateTexture(const rhi::TextureDesc& desc) {
    return rhi::TextureHandle{};
}

rhi::BufferHandle WebGPURHI::CreateBuffer(const rhi::BufferDesc& desc, const void* initialData) {
    return rhi::BufferHandle{};
}

void WebGPURHI::DestroySampler(rhi::SamplerHandle handle) {
    if (handle.id != 0xFFFFFFFFFFFFFFFF) {
        m_Samplers[handle.id] = {};
        handle.id = 0xFFFFFFFFFFFFFFFF;
    }
}

void WebGPURHI::DestroyTexture(rhi::TextureHandle handle) {
    return;
}

void WebGPURHI::DestroyBuffer(rhi::BufferHandle handle) {
    return;
}

void WebGPURHI::WriteBuffer(rhi::BufferHandle handle, const void* data, std::size_t size, std::size_t offset) {
    return;
}

void WebGPURHI::WriteTexture(rhi::TextureHandle handle, const rhi::TextureWriteDesc& desc, const void* data) {
    return;
}
