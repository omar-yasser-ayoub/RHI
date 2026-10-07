#include "rhi/webgpu/wgpurhi.h"

bool NeedsStaging(const rhi::BufferDesc& desc)
{
    if (desc.memoryUsage != rhi::MemoryUsage::CpuToGpu) {
        return false;
    }
    const rhi::BufferUsage bindMask =
        rhi::BufferUsage::Vertex | rhi::BufferUsage::Index | rhi::BufferUsage::Uniform |
        rhi::BufferUsage::Storage | rhi::BufferUsage::Indirect;
    return (desc.usage & bindMask) != rhi::BufferUsage::None;
}

rhi::BufferHandle WebGPURHI::CreateBuffer(const rhi::BufferDesc& desc) {

    rhi::BufferHandle handle;

    if (!m_Device || desc.size == 0) {
        printf("[RHI] Invalid WebGPU buffer description\n");
        return handle;
    }

    wgpu::BufferDescriptor bufferDesc{};
    bufferDesc.size = desc.size;
    if (NeedsStaging(desc)) {
        bufferDesc.usage = toWebGPU(desc.usage) | wgpu::BufferUsage::CopyDst;
    } else {
        bufferDesc.usage = toWebGPU(desc.usage) | toWebGPU(desc.memoryUsage);
    }

    wgpu::Buffer buffer = m_Device.CreateBuffer(&bufferDesc);
    if (!buffer) {
        printf("[RHI] Failed to create WebGPU buffer\n");
        return handle;
    }

    handle.desc = desc;
    if (!m_FreeBuffers.empty()) {
        handle.id = m_FreeBuffers.back();
        m_FreeBuffers.pop_back();
        m_Buffers[handle.id] = buffer;
    } else {
        handle.id = static_cast<uint64_t>(m_Buffers.size());
        m_Buffers.push_back(buffer);
    }

    if (NeedsStaging(desc)) {
        wgpu::BufferDescriptor stagingDesc{};
        stagingDesc.size = desc.size;
        stagingDesc.usage = wgpu::BufferUsage::MapWrite | wgpu::BufferUsage::CopySrc;
        wgpu::Buffer staging = m_Device.CreateBuffer(&stagingDesc);
        if (!staging) {
            printf("[RHI] Failed to create WebGPU staging buffer\n");
            m_Buffers[handle.id] = {};
            m_FreeBuffers.push_back(handle.id);
            return rhi::BufferHandle{};
        }
        m_StagingBuffers[handle.id] = staging;
    }

    return handle;
}

void* WebGPURHI::MapBuffer(rhi::BufferHandle handle) {
    if (handle.id == 0xFFFFFFFFFFFFFFFF ||
        handle.id >= m_Buffers.size() ||
        !m_Buffers[handle.id]) {
        return nullptr;
    }
    if (handle.desc.memoryUsage == rhi::MemoryUsage::GpuOnly) {
        printf("[RHI] Cannot map a GPU-only buffer\n");
        return nullptr;
    }
    const auto stagingIt = m_StagingBuffers.find(handle.id);
    if (stagingIt != m_StagingBuffers.end()) {
        wgpu::Buffer staging = stagingIt->second;
        bool success = false;
        auto future = staging.MapAsync(
            wgpu::MapMode::Write, 0, handle.desc.size,
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
            return nullptr;
        }
        m_StagingMapped[handle.id] = true;
        return staging.GetMappedRange(0, handle.desc.size);
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
        !m_Buffers[handle.id]) {
        return;
    }
    if (handle.desc.memoryUsage == rhi::MemoryUsage::GpuOnly) {
        printf("[RHI] Cannot unmap a GPU-only buffer\n");
        return;
    }
    const auto stagingIt = m_StagingBuffers.find(handle.id);
    if (stagingIt != m_StagingBuffers.end()) {
        const auto mappedIt = m_StagingMapped.find(handle.id);
        if (mappedIt == m_StagingMapped.end()) {
            return;
        }
        m_StagingMapped.erase(mappedIt);
        stagingIt->second.Unmap();

        wgpu::CommandEncoderDescriptor encoderDesc{};
        wgpu::CommandEncoder encoder = m_Device.CreateCommandEncoder(&encoderDesc);
        encoder.CopyBufferToBuffer(
            stagingIt->second, 0, m_Buffers[handle.id], 0, handle.desc.size);
        wgpu::CommandBuffer commands = encoder.Finish();
        m_Queue.Submit(1, &commands);
        return;
    }
    m_Buffers[handle.id].Unmap();
}

void WebGPURHI::WriteBuffer(rhi::BufferHandle handle, const void* data, std::size_t size, std::size_t offset) {
    if (!m_Device || !data || size == 0 ||
        handle.id == 0xFFFFFFFFFFFFFFFF ||
        handle.id >= m_Buffers.size() ||
        !m_Buffers[handle.id]) {
        return;
    }

    if (offset > handle.desc.size || size > handle.desc.size - offset) {
        printf("[RHI] WriteBuffer range is outside the WebGPU buffer\n");
        return;
    }
    if (handle.desc.memoryUsage == rhi::MemoryUsage::GpuToCpu) {
        printf("[RHI] Cannot write directly to a GPU-to-CPU WebGPU buffer\n");
        return;
    }

    if (handle.desc.memoryUsage == rhi::MemoryUsage::CpuToGpu &&
        m_StagingBuffers.find(handle.id) == m_StagingBuffers.end()) {
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

    if (m_StagingBuffers.find(handle.id) != m_StagingBuffers.end()) {
        void* mapped = MapBuffer(handle);
        if (!mapped) {
            return;
        }
        std::memcpy(static_cast<std::uint8_t*>(mapped) + offset, data, size);
        UnmapBuffer(handle);
        return;
    }

    m_Queue.WriteBuffer(m_Buffers[handle.id], offset, data, size);
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
    m_StagingBuffers.erase(handle.id);
    m_StagingMapped.erase(handle.id);
    m_FreeBuffers.push_back(handle.id);
}
