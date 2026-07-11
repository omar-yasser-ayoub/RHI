#include "rhi/webgpu/wgpurhi.h"
#include "rhi/irhi.h"
#include "rhi/resources/buffer.h"
#include <cstdio>
#include <cstring>
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
    m_BufferMetadata.clear();
    m_Samplers.clear();
    m_Queue = {};
    m_Device = {};
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

wgpu::BufferUsage WebGPURHI::toWebGPU(rhi::BufferUsage usage) {
    wgpu::BufferUsage result = wgpu::BufferUsage::None;
    if (HasFlag(usage, rhi::BufferUsage::Vertex)) result |= wgpu::BufferUsage::Vertex;
    if (HasFlag(usage, rhi::BufferUsage::Index)) result |= wgpu::BufferUsage::Index;
    if (HasFlag(usage, rhi::BufferUsage::Uniform)) result |= wgpu::BufferUsage::Uniform;
    if (HasFlag(usage, rhi::BufferUsage::Storage)) result |= wgpu::BufferUsage::Storage;
    if (HasFlag(usage, rhi::BufferUsage::Indirect)) result |= wgpu::BufferUsage::Indirect;
    if (HasFlag(usage, rhi::BufferUsage::CopySrc)) result |= wgpu::BufferUsage::CopySrc;
    if (HasFlag(usage, rhi::BufferUsage::CopyDst)) result |= wgpu::BufferUsage::CopyDst;
    return result;
}

wgpu::BufferUsage WebGPURHI::toWebGPU(rhi::MemoryUsage usage) {
    switch (usage) {
        case rhi::MemoryUsage::GpuOnly:
            return wgpu::BufferUsage::None;
        case rhi::MemoryUsage::CpuToGpu:
            return wgpu::BufferUsage::MapWrite | wgpu::BufferUsage::CopySrc;
        case rhi::MemoryUsage::GpuToCpu:
            return wgpu::BufferUsage::MapRead | wgpu::BufferUsage::CopyDst;
        default:
            return wgpu::BufferUsage::None;
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
    if (!sampler) {
        printf("[RHI] Failed to create WebGPU sampler\n");
        return handle;
    }
    handle.id = static_cast<uint64_t>(m_Samplers.size());
    m_Samplers.push_back(sampler);

    return handle;
}

rhi::TextureHandle WebGPURHI::CreateTexture(const rhi::TextureDesc& desc) {
    return rhi::TextureHandle{};
}

rhi::BufferHandle WebGPURHI::CreateBuffer(const rhi::BufferDesc& desc) {

    rhi::BufferHandle handle;

    if (!m_Device || desc.size == 0) {
        printf("[RHI] Invalid WebGPU buffer description\n");
        return handle;
    }

    if (desc.memoryUsage == rhi::MemoryUsage::CpuToGpu &&
        desc.usage != rhi::BufferUsage::None) {
        printf("[RHI] Invalid WebGPU buffer description: CpuToGpu buffers "
               "cannot be directly mapped with GPU usage flags\n");
        return handle;
    }

    wgpu::BufferDescriptor bufferDesc{};
    bufferDesc.size = desc.size;
    bufferDesc.usage = toWebGPU(desc.usage) | toWebGPU(desc.memoryUsage);

    wgpu::Buffer buffer = m_Device.CreateBuffer(&bufferDesc);
    if (!buffer) {
        printf("[RHI] Failed to create WebGPU buffer\n");
        return handle;
    }

    handle.id = static_cast<uint64_t>(m_Buffers.size());
    m_Buffers.push_back(buffer);
    m_BufferMetadata.push_back(desc);

    return handle;
}

void* WebGPURHI::MapBuffer(rhi::BufferHandle handle) {
    if (handle.id == 0xFFFFFFFFFFFFFFFF ||
        handle.id >= m_Buffers.size() ||
        handle.id >= m_BufferMetadata.size() ||
        !m_Buffers[handle.id]) {
        return nullptr;
    }
    if (m_BufferMetadata[handle.id].memoryUsage == rhi::MemoryUsage::GpuOnly) {
        printf("[RHI] Cannot map a GPU-only buffer\n");
        return nullptr;
    }
    wgpu::Buffer buffer = m_Buffers[handle.id];
    wgpu::BufferUsage usage = buffer.GetUsage();
    bool canWrite = (usage & wgpu::BufferUsage::MapWrite) != wgpu::BufferUsage::None;
    bool canRead  = (usage & wgpu::BufferUsage::MapRead)  != wgpu::BufferUsage::None;
    if (!canWrite && !canRead)
    {
        printf("[RHI] MapBuffer called on a buffer that is not mappable\n");
        return nullptr;
    }
    wgpu::MapMode mode = canWrite ? wgpu::MapMode::Write : wgpu::MapMode::Read;

    bool success = false;
    auto future = buffer.MapAsync(mode, 0, buffer.GetSize(), wgpu::CallbackMode::WaitAnyOnly,
            [&](wgpu::MapAsyncStatus status, wgpu::StringView message)
            {
                if (status != wgpu::MapAsyncStatus::Success) {
                    printf("[RHI] MapBuffer failed: %.*s\n",
                           static_cast<int>(message.length), message.data ? message.data : "");
                    return;
                }
                success = true;
            });

    m_Instance.WaitAny(future, UINT64_MAX);
    return success ? buffer.GetMappedRange(0, buffer.GetSize()) : nullptr;
}

void WebGPURHI::UnmapBuffer(rhi::BufferHandle handle) {
    if (handle.id == 0xFFFFFFFFFFFFFFFF ||
        handle.id >= m_Buffers.size() ||
        handle.id >= m_BufferMetadata.size() ||
        !m_Buffers[handle.id]) {
        return;
    }
    if (m_BufferMetadata[handle.id].memoryUsage == rhi::MemoryUsage::GpuOnly) {
        printf("[RHI] Cannot unmap a GPU-only buffer\n");
        return;
    }
    m_Buffers[handle.id].Unmap();
}

void WebGPURHI::DestroySampler(rhi::SamplerHandle handle) {
    if (handle.id == 0xFFFFFFFFFFFFFFFF || handle.id >= m_Samplers.size()) {
        return;
    }
    m_Samplers[handle.id] = {};
}

void WebGPURHI::DestroyTexture(rhi::TextureHandle handle) {
    return;
}

void WebGPURHI::DestroyBuffer(rhi::BufferHandle handle)
{
    if (handle.id == 0xFFFFFFFFFFFFFFFF ||
        handle.id >= m_Buffers.size() ||
        !m_Buffers[handle.id]) {
        return;
    }
    m_Buffers[handle.id].Unmap();
    m_Buffers[handle.id].Destroy();
    m_Buffers[handle.id] = {};
    if (handle.id < m_BufferMetadata.size()) {
        m_BufferMetadata[handle.id] = {};
    }
}

void WebGPURHI::WriteBuffer(rhi::BufferHandle handle, const void* data, std::size_t size, std::size_t offset) {
    if (!m_Device || !data || size == 0 ||
        handle.id == 0xFFFFFFFFFFFFFFFF ||
        handle.id >= m_Buffers.size() ||
        handle.id >= m_BufferMetadata.size() ||
        !m_Buffers[handle.id]) {
        return;
    }

    const rhi::BufferDesc& metadata = m_BufferMetadata[handle.id];
    if (offset > metadata.size || size > metadata.size - offset) {
        printf("[RHI] WriteBuffer range is outside the WebGPU buffer\n");
        return;
    }
    if (metadata.memoryUsage == rhi::MemoryUsage::GpuToCpu) {
        printf("[RHI] Cannot write directly to a GPU-to-CPU WebGPU buffer\n");
        return;
    }

    if (metadata.memoryUsage == rhi::MemoryUsage::CpuToGpu) {
        wgpu::Buffer mappedBuffer = m_Buffers[handle.id];
        bool success = false;
        auto future = mappedBuffer.MapAsync(
            wgpu::MapMode::Write,
            offset,
            size,
            wgpu::CallbackMode::WaitAnyOnly,
            [&](wgpu::MapAsyncStatus status, wgpu::StringView message) {
                if (status != wgpu::MapAsyncStatus::Success) {
                    printf("[RHI] MapBuffer failed: %.*s\n",
                           static_cast<int>(message.length),
                           message.data ? message.data : "");
                    return;
                }
                success = true;
            });
        m_Instance.WaitAny(future, UINT64_MAX);
        if (!success) {
            return;
        }

        void* mapped = mappedBuffer.GetMappedRange(offset, size);
        if (!mapped) {
            printf("[RHI] Failed to get mapped WebGPU buffer range\n");
            mappedBuffer.Unmap();
            return;
        }
        std::memcpy(mapped, data, size);
        mappedBuffer.Unmap();
        return;
    }

    m_Queue.WriteBuffer(m_Buffers[handle.id], offset, data, size);
}

void WebGPURHI::WriteTexture(rhi::TextureHandle handle, const rhi::TextureWriteDesc& desc, const void* data) {
    return;
}
