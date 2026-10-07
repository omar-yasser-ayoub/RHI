#include "rhi/webgpu/wgpurhi.h"

rhi::BufferHandle WebGPURHI::CreateBuffer(const rhi::BufferDesc& desc) {

    rhi::BufferHandle handle;

    if (!m_Device || desc.size == 0) {
        printf("[RHI] Invalid WebGPU buffer description\n");
        return handle;
    }

    if (desc.memoryUsage != rhi::MemoryUsage::GpuOnly) {
        const rhi::BufferUsage bindMask =
            rhi::BufferUsage::Vertex | rhi::BufferUsage::Index | rhi::BufferUsage::Uniform |
            rhi::BufferUsage::Storage | rhi::BufferUsage::Indirect;
        if ((desc.usage & bindMask) != rhi::BufferUsage::None) {
            printf("[RHI] CpuToGpu/GpuToCpu buffers cannot bind to the pipeline; stage explicitly\n");
            return handle;
        }
    }

    wgpu::BufferDescriptor bufferDesc{};
    bufferDesc.size = desc.size;
    bufferDesc.usage = toWebGPU(desc.usage) | toWebGPU(desc.memoryUsage);

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

    if (handle.desc.memoryUsage == rhi::MemoryUsage::CpuToGpu) {
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
    m_FreeBuffers.push_back(handle.id);
}
