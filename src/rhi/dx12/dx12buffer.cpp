#include "rhi/dx12/dx12rhi.h"
#include <cstring>

D3D12_RESOURCE_STATES DX12RHI::toD3D12InitialState(
    rhi::MemoryUsage memoryUsage,
    rhi::BufferUsage bufferUsage) {
    switch (memoryUsage)
    {
        case rhi::MemoryUsage::CpuToGpu:
            return D3D12_RESOURCE_STATE_GENERIC_READ;
        case rhi::MemoryUsage::GpuToCpu:
            return D3D12_RESOURCE_STATE_COPY_DEST;
        case rhi::MemoryUsage::GpuOnly:
            break;
        default:
            return D3D12_RESOURCE_STATE_COMMON;
    }

    D3D12_RESOURCE_STATES state = D3D12_RESOURCE_STATE_COMMON;
    if (rhi::HasFlag(bufferUsage, rhi::BufferUsage::Vertex) ||
        rhi::HasFlag(bufferUsage, rhi::BufferUsage::Uniform)) {
        state |= D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
    }
    if (rhi::HasFlag(bufferUsage, rhi::BufferUsage::Index)) {
        state |= D3D12_RESOURCE_STATE_INDEX_BUFFER;
    }
    if (rhi::HasFlag(bufferUsage, rhi::BufferUsage::Storage)) {
        state |= D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
    }
    if (rhi::HasFlag(bufferUsage, rhi::BufferUsage::Indirect)) {
        state |= D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT;
    }
    if (rhi::HasFlag(bufferUsage, rhi::BufferUsage::CopySrc)) {
        state |= D3D12_RESOURCE_STATE_COPY_SOURCE;
    }
    if (rhi::HasFlag(bufferUsage, rhi::BufferUsage::CopyDst)) {
        state |= D3D12_RESOURCE_STATE_COPY_DEST;
    }

    return state;
}

rhi::BufferHandle DX12RHI::CreateBuffer(const rhi::BufferDesc& desc) {
    if (!m_Device || desc.size == 0) {
        printf("[ERROR] Invalid buffer description\n");
        return rhi::BufferHandle{};
    }

    D3D12_HEAP_PROPERTIES heapProps = toD3D12(desc.memoryUsage);
    D3D12_RESOURCE_STATES initialState = toD3D12InitialState(
        desc.memoryUsage, desc.usage);

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
    HRESULT result = m_Device->CreateCommittedResource(
        &heapProps,
        D3D12_HEAP_FLAG_NONE,
        &resourceDesc,
        initialState,
        nullptr,
        IID_PPV_ARGS(&resource));
    if (FAILED(result)) {
        printf("[ERROR] Failed to create buffer: 0x%08X\n",
               static_cast<unsigned>(result));
        return rhi::BufferHandle{};
    }

    rhi::BufferHandle handle;
    handle.id = static_cast<uint64_t>(m_Buffers.size());
    handle.desc = desc;
    m_Buffers.push_back(std::move(resource));
    return handle;
}



void DX12RHI::DestroyBuffer(rhi::BufferHandle handle)
{
    if (!m_Device || handle.id == 0xFFFFFFFFFFFFFFFF ||
        handle.id >= m_Buffers.size() ||
        !m_Buffers[handle.id]) {
        return;
    }
    m_Buffers[handle.id].Reset();
}

void* DX12RHI::MapBuffer(rhi::BufferHandle handle) {
    if (!m_Device || handle.id == 0xFFFFFFFFFFFFFFFF ||
        handle.id >= m_Buffers.size() ||
        !m_Buffers[handle.id]) {
        return nullptr;
    }
    if (handle.desc.memoryUsage == rhi::MemoryUsage::GpuOnly) {
        printf("[ERROR] Cannot map a GPU-only buffer\n");
        return nullptr;
    }

    void* ptr = nullptr;
    D3D12_RANGE readRange{};
    if (handle.desc.memoryUsage == rhi::MemoryUsage::GpuToCpu) {
        readRange.End = static_cast<SIZE_T>(handle.desc.size);
    }

    if (FAILED(m_Buffers[handle.id]->Map(0, &readRange, &ptr)))
        {
            printf("[ERROR] Failed to map buffer\n");
            return nullptr;
        }

    return ptr;
}

void DX12RHI::UnmapBuffer(rhi::BufferHandle handle) {
    if (!m_Device || handle.id == 0xFFFFFFFFFFFFFFFF ||
        handle.id >= m_Buffers.size() ||
        !m_Buffers[handle.id]) {
        return;
    }
    if (handle.desc.memoryUsage == rhi::MemoryUsage::GpuOnly) {
        printf("[ERROR] Cannot unmap a GPU-only buffer\n");
        return;
    }
    m_Buffers[handle.id]->Unmap(0, nullptr);
}

void DX12RHI::WriteBuffer(rhi::BufferHandle handle, const void* data, std::size_t size, std::size_t offset) {
    if (!m_Device || !data || size == 0 ||
        handle.id == 0xFFFFFFFFFFFFFFFF ||
        handle.id >= m_Buffers.size() ||
        !m_Buffers[handle.id]) {
        return;
    }

    if (offset > handle.desc.size || size > handle.desc.size - offset) {
        printf("[ERROR] WriteBuffer range is outside the DX12 buffer\n");
        return;
    }
    if (handle.desc.memoryUsage == rhi::MemoryUsage::GpuOnly) {
        printf("[ERROR] GPU-only DX12 buffer writes require an upload buffer\n");
        return;
    }

    void* mapped = nullptr;
    D3D12_RANGE readRange{};
    HRESULT result = m_Buffers[handle.id]->Map(0, &readRange, &mapped);
    if (FAILED(result)) {
        printf("[ERROR] Failed to map DX12 buffer for writing: 0x%08X\n",
               static_cast<unsigned>(result));
        return;
    }

    std::memcpy(static_cast<std::byte*>(mapped) + offset, data, size);
    D3D12_RANGE writtenRange{
        static_cast<SIZE_T>(offset),
        static_cast<SIZE_T>(offset + size)
    };
    m_Buffers[handle.id]->Unmap(0, &writtenRange);
}
