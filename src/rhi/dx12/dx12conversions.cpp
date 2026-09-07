#include "rhi/dx12/dx12rhi.h"
#include "rhi/resources/texture.h"
#include <array>
#include <d3d12.h>

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

D3D12_RESOURCE_DIMENSION DX12RHI::toD3D12(rhi::TextureDimension dimension) {
    switch (dimension)
    {
        case rhi::TextureDimension::Tex1D: return D3D12_RESOURCE_DIMENSION_TEXTURE1D;
        case rhi::TextureDimension::Tex2D: return D3D12_RESOURCE_DIMENSION_TEXTURE2D;
        case rhi::TextureDimension::Tex3D: return D3D12_RESOURCE_DIMENSION_TEXTURE3D;
        default: return D3D12_RESOURCE_DIMENSION_UNKNOWN;
    }
}

DXGI_FORMAT DX12RHI::toD3D12(rhi::TextureFormat format)
{
    switch (format)
    {
        case rhi::TextureFormat::Unknown:              return DXGI_FORMAT_UNKNOWN;
        case rhi::TextureFormat::R8Unorm:              return DXGI_FORMAT_R8_UNORM;
        case rhi::TextureFormat::R8Snorm:              return DXGI_FORMAT_R8_SNORM;
        case rhi::TextureFormat::R8Uint:               return DXGI_FORMAT_R8_UINT;
        case rhi::TextureFormat::R8Sint:               return DXGI_FORMAT_R8_SINT;
        case rhi::TextureFormat::RG8Unorm:             return DXGI_FORMAT_R8G8_UNORM;
        case rhi::TextureFormat::RG8Snorm:             return DXGI_FORMAT_R8G8_SNORM;
        case rhi::TextureFormat::RG8Uint:              return DXGI_FORMAT_R8G8_UINT;
        case rhi::TextureFormat::RG8Sint:              return DXGI_FORMAT_R8G8_SINT;
        case rhi::TextureFormat::RGBA8Unorm:           return DXGI_FORMAT_R8G8B8A8_UNORM;
        case rhi::TextureFormat::RGBA8UnormSrgb:       return DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
        case rhi::TextureFormat::RGBA8Snorm:           return DXGI_FORMAT_R8G8B8A8_SNORM;
        case rhi::TextureFormat::RGBA8Uint:            return DXGI_FORMAT_R8G8B8A8_UINT;
        case rhi::TextureFormat::RGBA8Sint:            return DXGI_FORMAT_R8G8B8A8_SINT;
        case rhi::TextureFormat::BGRA8Unorm:           return DXGI_FORMAT_B8G8R8A8_UNORM;
        case rhi::TextureFormat::BGRA8UnormSrgb:       return DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
        case rhi::TextureFormat::R16Unorm:             return DXGI_FORMAT_R16_UNORM;
        case rhi::TextureFormat::R16Snorm:             return DXGI_FORMAT_R16_SNORM;
        case rhi::TextureFormat::R16Uint:              return DXGI_FORMAT_R16_UINT;
        case rhi::TextureFormat::R16Sint:              return DXGI_FORMAT_R16_SINT;
        case rhi::TextureFormat::R16Float:             return DXGI_FORMAT_R16_FLOAT;
        case rhi::TextureFormat::RG16Unorm:            return DXGI_FORMAT_R16G16_UNORM;
        case rhi::TextureFormat::RG16Snorm:            return DXGI_FORMAT_R16G16_SNORM;
        case rhi::TextureFormat::RG16Uint:             return DXGI_FORMAT_R16G16_UINT;
        case rhi::TextureFormat::RG16Sint:             return DXGI_FORMAT_R16G16_SINT;
        case rhi::TextureFormat::RG16Float:            return DXGI_FORMAT_R16G16_FLOAT;
        case rhi::TextureFormat::RGBA16Unorm:          return DXGI_FORMAT_R16G16B16A16_UNORM;
        case rhi::TextureFormat::RGBA16Snorm:          return DXGI_FORMAT_R16G16B16A16_SNORM;
        case rhi::TextureFormat::RGBA16Uint:           return DXGI_FORMAT_R16G16B16A16_UINT;
        case rhi::TextureFormat::RGBA16Sint:           return DXGI_FORMAT_R16G16B16A16_SINT;
        case rhi::TextureFormat::RGBA16Float:          return DXGI_FORMAT_R16G16B16A16_FLOAT;
        case rhi::TextureFormat::R32Uint:              return DXGI_FORMAT_R32_UINT;
        case rhi::TextureFormat::R32Sint:              return DXGI_FORMAT_R32_SINT;
        case rhi::TextureFormat::R32Float:             return DXGI_FORMAT_R32_FLOAT;
        case rhi::TextureFormat::RG32Uint:             return DXGI_FORMAT_R32G32_UINT;
        case rhi::TextureFormat::RG32Sint:             return DXGI_FORMAT_R32G32_SINT;
        case rhi::TextureFormat::RG32Float:            return DXGI_FORMAT_R32G32_FLOAT;
        case rhi::TextureFormat::RGB32Uint:            return DXGI_FORMAT_R32G32B32_UINT;
        case rhi::TextureFormat::RGB32Sint:            return DXGI_FORMAT_R32G32B32_SINT;
        case rhi::TextureFormat::RGB32Float:           return DXGI_FORMAT_R32G32B32_FLOAT;
        case rhi::TextureFormat::RGBA32Uint:           return DXGI_FORMAT_R32G32B32A32_UINT;
        case rhi::TextureFormat::RGBA32Sint:           return DXGI_FORMAT_R32G32B32A32_SINT;
        case rhi::TextureFormat::RGBA32Float:          return DXGI_FORMAT_R32G32B32A32_FLOAT;
        case rhi::TextureFormat::RGB10A2Unorm:         return DXGI_FORMAT_R10G10B10A2_UNORM;
        case rhi::TextureFormat::RGB10A2Uint:          return DXGI_FORMAT_R10G10B10A2_UINT;
        case rhi::TextureFormat::RG11B10Float:         return DXGI_FORMAT_R11G11B10_FLOAT;
        case rhi::TextureFormat::RGB9E5Float:          return DXGI_FORMAT_R9G9B9E5_SHAREDEXP;

        // ---------------- Depth / Stencil ----------------
        // NOTE: these are the *view* (typed) formats. A depth texture that will
        // also be sampled must be created with the matching TYPELESS resource
        // format instead - see toD3D12Typeless() / toD3D12DepthSrv() below.
        case rhi::TextureFormat::Depth16Unorm:         return DXGI_FORMAT_D16_UNORM;
        case rhi::TextureFormat::Depth24Plus:          return DXGI_FORMAT_D32_FLOAT;
        case rhi::TextureFormat::Depth24PlusStencil8:  return DXGI_FORMAT_D24_UNORM_S8_UINT;
        case rhi::TextureFormat::Depth32Float:         return DXGI_FORMAT_D32_FLOAT;
        case rhi::TextureFormat::Depth32FloatStencil8: return DXGI_FORMAT_D32_FLOAT_S8X24_UINT;
        case rhi::TextureFormat::Stencil8:             return DXGI_FORMAT_D24_UNORM_S8_UINT;
        case rhi::TextureFormat::BC1RGBAUnorm:         return DXGI_FORMAT_BC1_UNORM;
        case rhi::TextureFormat::BC1RGBAUnormSrgb:     return DXGI_FORMAT_BC1_UNORM_SRGB;
        case rhi::TextureFormat::BC2RGBAUnorm:         return DXGI_FORMAT_BC2_UNORM;
        case rhi::TextureFormat::BC2RGBAUnormSrgb:     return DXGI_FORMAT_BC2_UNORM_SRGB;
        case rhi::TextureFormat::BC3RGBAUnorm:         return DXGI_FORMAT_BC3_UNORM;
        case rhi::TextureFormat::BC3RGBAUnormSrgb:     return DXGI_FORMAT_BC3_UNORM_SRGB;
        case rhi::TextureFormat::BC4RUnorm:            return DXGI_FORMAT_BC4_UNORM;
        case rhi::TextureFormat::BC4RSnorm:            return DXGI_FORMAT_BC4_SNORM;
        case rhi::TextureFormat::BC5RGUnorm:           return DXGI_FORMAT_BC5_UNORM;
        case rhi::TextureFormat::BC5RGSnorm:           return DXGI_FORMAT_BC5_SNORM;
        case rhi::TextureFormat::BC6HRGBUFloat:        return DXGI_FORMAT_BC6H_UF16;
        case rhi::TextureFormat::BC6HRGBFloat:         return DXGI_FORMAT_BC6H_SF16;
        case rhi::TextureFormat::BC7RGBAUnorm:         return DXGI_FORMAT_BC7_UNORM;
        case rhi::TextureFormat::BC7RGBAUnormSrgb:     return DXGI_FORMAT_BC7_UNORM_SRGB;

        // No DXGI equivalents. D3D12 never shipped ETC2 support
        case rhi::TextureFormat::ETC2RGB8Unorm:
        case rhi::TextureFormat::ETC2RGB8UnormSrgb:
        case rhi::TextureFormat::ETC2RGB8A1Unorm:
        case rhi::TextureFormat::ETC2RGB8A1UnormSrgb:
        case rhi::TextureFormat::ETC2RGBA8Unorm:
        case rhi::TextureFormat::ETC2RGBA8UnormSrgb:
        case rhi::TextureFormat::EACR11Unorm:
        case rhi::TextureFormat::EACR11Snorm:
        case rhi::TextureFormat::EACRG11Unorm:
        case rhi::TextureFormat::EACRG11Snorm:
            return DXGI_FORMAT_UNKNOWN;

        // DXGI reserved enum values 133-188 for ASTC but they were never
        // published in the public header and no D3D12 driver exposes them.
        // Treat as unsupported and transcode to BC7.
        case rhi::TextureFormat::ASTC4x4Unorm:
        case rhi::TextureFormat::ASTC4x4UnormSrgb:
        case rhi::TextureFormat::ASTC5x4Unorm:
        case rhi::TextureFormat::ASTC5x4UnormSrgb:
        case rhi::TextureFormat::ASTC5x5Unorm:
        case rhi::TextureFormat::ASTC5x5UnormSrgb:
        case rhi::TextureFormat::ASTC6x5Unorm:
        case rhi::TextureFormat::ASTC6x5UnormSrgb:
        case rhi::TextureFormat::ASTC6x6Unorm:
        case rhi::TextureFormat::ASTC6x6UnormSrgb:
        case rhi::TextureFormat::ASTC8x5Unorm:
        case rhi::TextureFormat::ASTC8x5UnormSrgb:
        case rhi::TextureFormat::ASTC8x6Unorm:
        case rhi::TextureFormat::ASTC8x6UnormSrgb:
        case rhi::TextureFormat::ASTC8x8Unorm:
        case rhi::TextureFormat::ASTC8x8UnormSrgb:
        case rhi::TextureFormat::ASTC10x5Unorm:
        case rhi::TextureFormat::ASTC10x5UnormSrgb:
        case rhi::TextureFormat::ASTC10x6Unorm:
        case rhi::TextureFormat::ASTC10x6UnormSrgb:
        case rhi::TextureFormat::ASTC10x8Unorm:
        case rhi::TextureFormat::ASTC10x8UnormSrgb:
        case rhi::TextureFormat::ASTC10x10Unorm:
        case rhi::TextureFormat::ASTC10x10UnormSrgb:
        case rhi::TextureFormat::ASTC12x10Unorm:
        case rhi::TextureFormat::ASTC12x10UnormSrgb:
        case rhi::TextureFormat::ASTC12x12Unorm:
        case rhi::TextureFormat::ASTC12x12UnormSrgb:
            return DXGI_FORMAT_UNKNOWN;
    }
}



DXGI_FORMAT DX12RHI::toD3D12Typeless(rhi::TextureFormat format)
{
    switch (format)
    {
        case rhi::TextureFormat::Depth16Unorm:         return DXGI_FORMAT_R16_TYPELESS;
        case rhi::TextureFormat::Depth24Plus:          return DXGI_FORMAT_R32_TYPELESS;
        case rhi::TextureFormat::Depth24PlusStencil8:  return DXGI_FORMAT_R24G8_TYPELESS;
        case rhi::TextureFormat::Depth32Float:         return DXGI_FORMAT_R32_TYPELESS;
        case rhi::TextureFormat::Depth32FloatStencil8: return DXGI_FORMAT_R32G8X24_TYPELESS;
        case rhi::TextureFormat::Stencil8:             return DXGI_FORMAT_R24G8_TYPELESS;
        default:                                       return toD3D12(format);
    }
}

// SRV format for the depth plane (plane slice 0).
DXGI_FORMAT DX12RHI::toD3D12DepthSrv(rhi::TextureFormat format)
{
    switch (format)
    {
        case rhi::TextureFormat::Depth16Unorm:         return DXGI_FORMAT_R16_UNORM;
        case rhi::TextureFormat::Depth24Plus:          return DXGI_FORMAT_R32_FLOAT;
        case rhi::TextureFormat::Depth24PlusStencil8:  return DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
        case rhi::TextureFormat::Depth32Float:         return DXGI_FORMAT_R32_FLOAT;
        case rhi::TextureFormat::Depth32FloatStencil8: return DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS;
        default:                                       return DXGI_FORMAT_UNKNOWN;
    }
}

// SRV format for the stencil plane (plane slice 1).
DXGI_FORMAT DX12RHI::toD3D12StencilSrv(rhi::TextureFormat format)
{
    switch (format)
    {
        case rhi::TextureFormat::Depth24PlusStencil8:  return DXGI_FORMAT_X24_TYPELESS_G8_UINT;
        case rhi::TextureFormat::Depth32FloatStencil8: return DXGI_FORMAT_X32_TYPELESS_G8X24_UINT;
        case rhi::TextureFormat::Stencil8:             return DXGI_FORMAT_X24_TYPELESS_G8_UINT;
        default:                                       return DXGI_FORMAT_UNKNOWN;
    }
}

uint32_t DX12RHI::toD3D12(rhi::SampleCount count) {
    switch (count)
    {
        case rhi::SampleCount::x1: return 1;
        case rhi::SampleCount::x2: return 2;
        case rhi::SampleCount::x4: return 4;
        case rhi::SampleCount::x8: return 8;
        case rhi::SampleCount::x16: return 16;
        default: return 1;
    }
}

D3D12_RESOURCE_FLAGS DX12RHI::toD3D12(rhi::TextureUsage usage) {
    D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE;

    if ((usage & rhi::TextureUsage::RenderTarget) != rhi::TextureUsage::None)
        flags |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

    if ((usage & rhi::TextureUsage::DepthStencil) != rhi::TextureUsage::None)
        flags |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

    if ((usage & rhi::TextureUsage::Storage) != rhi::TextureUsage::None)
        flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

    return flags;
}
bool DX12RHI::IsDepthFormat(rhi::TextureFormat format)
{
    switch (format)
    {
        case rhi::TextureFormat::Depth16Unorm:
        case rhi::TextureFormat::Depth24Plus:
        case rhi::TextureFormat::Depth24PlusStencil8:
        case rhi::TextureFormat::Depth32Float:
        case rhi::TextureFormat::Depth32FloatStencil8:
        case rhi::TextureFormat::Stencil8:
            return true;
        default:
            return false;
    }
}

D3D12_SRV_DIMENSION DX12RHI::toD3D12SrvDimension(rhi::TextureViewDimension dimension)
{
    switch (dimension)
    {
        case rhi::TextureViewDimension::Tex1D: return D3D12_SRV_DIMENSION_TEXTURE1D;
        case rhi::TextureViewDimension::Tex2D: return D3D12_SRV_DIMENSION_TEXTURE2D;
        case rhi::TextureViewDimension::Tex3D: return D3D12_SRV_DIMENSION_TEXTURE3D;
        case rhi::TextureViewDimension::Tex1DArray: return D3D12_SRV_DIMENSION_TEXTURE1DARRAY;
        case rhi::TextureViewDimension::Tex2DArray: return D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
        case rhi::TextureViewDimension::TexCube: return D3D12_SRV_DIMENSION_TEXTURECUBE;
        case rhi::TextureViewDimension::TexCubeArray: return D3D12_SRV_DIMENSION_TEXTURECUBEARRAY;
        default: return D3D12_SRV_DIMENSION_UNKNOWN;
    }
}

D3D12_UAV_DIMENSION DX12RHI::toD3D12UavDimension(rhi::TextureViewDimension dimension)
{
    switch (dimension)
    {
        case rhi::TextureViewDimension::Tex1D:      return D3D12_UAV_DIMENSION_TEXTURE1D;
        case rhi::TextureViewDimension::Tex2D:      return D3D12_UAV_DIMENSION_TEXTURE2D;
        case rhi::TextureViewDimension::Tex3D:      return D3D12_UAV_DIMENSION_TEXTURE3D;
        case rhi::TextureViewDimension::Tex1DArray: return D3D12_UAV_DIMENSION_TEXTURE1DARRAY;
        case rhi::TextureViewDimension::Tex2DArray: return D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
        case rhi::TextureViewDimension::TexCube:
        case rhi::TextureViewDimension::TexCubeArray:
            return D3D12_UAV_DIMENSION_TEXTURE2DARRAY;

        default: return D3D12_UAV_DIMENSION_UNKNOWN;
    }
}

D3D12_RTV_DIMENSION DX12RHI::toD3D12RtvDimension(rhi::TextureViewDimension dimension)
{
    switch (dimension)
    {
        case rhi::TextureViewDimension::Tex1D:      return D3D12_RTV_DIMENSION_TEXTURE1D;
        case rhi::TextureViewDimension::Tex2D:      return D3D12_RTV_DIMENSION_TEXTURE2D;
        case rhi::TextureViewDimension::Tex3D:      return D3D12_RTV_DIMENSION_TEXTURE3D;
        case rhi::TextureViewDimension::Tex1DArray: return D3D12_RTV_DIMENSION_TEXTURE1DARRAY;
        case rhi::TextureViewDimension::Tex2DArray: return D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
        case rhi::TextureViewDimension::TexCube:
        case rhi::TextureViewDimension::TexCubeArray:
            return D3D12_RTV_DIMENSION_TEXTURE2DARRAY;

        default: return D3D12_RTV_DIMENSION_UNKNOWN;
    }
}
D3D12_DSV_DIMENSION DX12RHI::toD3D12DsvDimension(rhi::TextureViewDimension dimension)
{
    switch (dimension)
    {
        case rhi::TextureViewDimension::Tex1D:      return D3D12_DSV_DIMENSION_TEXTURE1D;
        case rhi::TextureViewDimension::Tex2D:      return D3D12_DSV_DIMENSION_TEXTURE2D;
        case rhi::TextureViewDimension::Tex1DArray: return D3D12_DSV_DIMENSION_TEXTURE1DARRAY;
        case rhi::TextureViewDimension::Tex2DArray: return D3D12_DSV_DIMENSION_TEXTURE2DARRAY;
        case rhi::TextureViewDimension::TexCube:
        case rhi::TextureViewDimension::TexCubeArray:
            return D3D12_DSV_DIMENSION_TEXTURE2DARRAY;

        default: return D3D12_DSV_DIMENSION_UNKNOWN;
    }
}

DX12RHI::DescriptorHandles DX12RHI::AllocateDescriptor(
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>& heap,
    uint32_t& offset,
    uint32_t descriptorSize)
{
    DescriptorHandles handles{};

    if (!heap || descriptorSize == 0 || offset >= heap->GetDesc().NumDescriptors) {
        return handles;
    }

    handles.cpu = heap->GetCPUDescriptorHandleForHeapStart();
    handles.cpu.ptr += static_cast<SIZE_T>(offset) * descriptorSize;

    if (heap->GetDesc().Flags & D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE)
    {
        handles.gpu = heap->GetGPUDescriptorHandleForHeapStart();
        handles.gpu.ptr += static_cast<UINT64>(offset) * descriptorSize;
    }

    ++offset;
    handles.valid = true;

    return handles;
}
