#include "rhi/dx12/dx12rhi.h"
#include <array>

D3D12_FILTER DX12RHI::toD3D12(rhi::Filter min, rhi::Filter mag, rhi::MipmapFilter mip, bool anisotropic, bool compare) {
    uint32_t filter = 0;

    if (anisotropic)                                filter |= 0x54;
    else
        {
            if (min == rhi::Filter::Linear)       filter |= 0x10;
            if (mag == rhi::Filter::Linear)       filter |= 0x04;
        }
    if (mip == rhi::MipmapFilter::Linear)     filter |= 0x01;
    if (compare)                              filter |= 0x80;

    return static_cast<D3D12_FILTER>(filter);
}

D3D12_TEXTURE_ADDRESS_MODE DX12RHI::toD3D12(rhi::AddressMode mode) {
    switch (mode) {
        case rhi::AddressMode::Repeat: return D3D12_TEXTURE_ADDRESS_MODE_WRAP;
        case rhi::AddressMode::MirrorRepeat: return D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
        case rhi::AddressMode::ClampToEdge: return D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
        case rhi::AddressMode::ClampToBorder: return D3D12_TEXTURE_ADDRESS_MODE_BORDER;
        case rhi::AddressMode::MirrorClampToEdge: return D3D12_TEXTURE_ADDRESS_MODE_MIRROR_ONCE;
        default: return D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    }
}

D3D12_COMPARISON_FUNC DX12RHI::toD3D12(rhi::CompareOp mode) {
    switch (mode) {
        case rhi::CompareOp::Never: return D3D12_COMPARISON_FUNC_NEVER;
        case rhi::CompareOp::Less: return D3D12_COMPARISON_FUNC_LESS;
        case rhi::CompareOp::Equal: return D3D12_COMPARISON_FUNC_EQUAL;
        case rhi::CompareOp::LessEqual: return D3D12_COMPARISON_FUNC_LESS_EQUAL;
        case rhi::CompareOp::Greater: return D3D12_COMPARISON_FUNC_GREATER;
        case rhi::CompareOp::NotEqual: return D3D12_COMPARISON_FUNC_NOT_EQUAL;
        case rhi::CompareOp::GreaterEqual: return D3D12_COMPARISON_FUNC_GREATER_EQUAL;
        case rhi::CompareOp::Always: return D3D12_COMPARISON_FUNC_ALWAYS;
        default: return D3D12_COMPARISON_FUNC_NONE;
    }
}

std::array<float, 4> DX12RHI::toD3D12(rhi::BorderColor color) {
    switch (color) {
        case rhi::BorderColor::TransparentBlack: return { 0.0f, 0.0f, 0.0f, 0.0f };
        case rhi::BorderColor::OpaqueBlack: return { 0.0f, 0.0f, 0.0f, 1.0f };
        case rhi::BorderColor::OpaqueWhite: return { 1.0f, 1.0f, 1.0f, 1.0f };
        default: return { 0.0f, 0.0f, 0.0f, 0.0f };
    }
}

D3D12_HEAP_PROPERTIES DX12RHI::toD3D12(rhi::MemoryUsage usage) {
    switch (usage)
    {
        case rhi::MemoryUsage::GpuOnly:  return D3D12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
        case rhi::MemoryUsage::CpuToGpu: return D3D12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
        case rhi::MemoryUsage::GpuToCpu: return D3D12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_READBACK);
        default:                         return D3D12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
    }
}
