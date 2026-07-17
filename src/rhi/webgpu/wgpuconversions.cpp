#include "rhi/webgpu/wgpurhi.h"

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
