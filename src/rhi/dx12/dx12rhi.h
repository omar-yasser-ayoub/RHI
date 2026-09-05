#pragma once

#include "rhi/irhi.h"
#include "rhi/resources/texture.h"
#include "rhi/resources/shader.h"


#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl/client.h>
#include <vector>
#include <third_party/dxc/inc/dxcapi.h>

class DX12RHI : public IRHI
{
public:
    bool Initialize();
    void Shutdown();

    rhi::SamplerHandle CreateSampler(const rhi::SamplerDesc& desc);
    rhi::TextureHandle CreateTexture(const rhi::TextureDesc& desc);
    rhi::BufferHandle CreateBuffer(const rhi::BufferDesc& desc);
    rhi::TextureViewHandle CreateTextureView(const rhi::TextureViewDesc& desc, rhi::TextureHandle handle);

    void DestroySampler(rhi::SamplerHandle handle);
    void DestroyTexture(rhi::TextureHandle handle);
    void DestroyBuffer(rhi::BufferHandle handle);
    void DestroyTextureView(rhi::TextureViewHandle handle);


    void WriteBuffer(rhi::BufferHandle handle, const void* data, std::size_t size, std::size_t offset = 0);
    void WriteTexture(rhi::TextureHandle handle, const rhi::TextureWriteDesc& desc, const void* data);

    void* MapBuffer(rhi::BufferHandle handle);
    void UnmapBuffer(rhi::BufferHandle handle);

    rhi::ShaderModule CompileShader(rhi::ShaderSource source);

    static D3D12_FILTER toD3D12(rhi::Filter min, rhi::Filter mag, rhi::MipmapFilter mip, bool anisotropic, bool compare);
    static D3D12_TEXTURE_ADDRESS_MODE toD3D12(rhi::AddressMode mode);
    static D3D12_COMPARISON_FUNC toD3D12(rhi::CompareOp mode);
    static std::array<float, 4> toD3D12(rhi::BorderColor color);
    static D3D12_HEAP_PROPERTIES toD3D12(rhi::MemoryUsage usage);
    static D3D12_RESOURCE_STATES toD3D12InitialState(
        rhi::MemoryUsage memoryUsage,
        rhi::BufferUsage bufferUsage);
    static D3D12_RESOURCE_DIMENSION toD3D12(rhi::TextureDimension dimension);
    static DXGI_FORMAT toD3D12(rhi::TextureFormat format);
    static uint32_t toD3D12(rhi::SampleCount count);
    static D3D12_RESOURCE_FLAGS toD3D12(rhi::TextureUsage usage);
    static DXGI_FORMAT toD3D12Typeless(rhi::TextureFormat format);
    static DXGI_FORMAT toD3D12DepthSrv(rhi::TextureFormat format);
    static DXGI_FORMAT toD3D12StencilSrv(rhi::TextureFormat format);
    static bool IsDepthFormat(rhi::TextureFormat format);
    static D3D12_SRV_DIMENSION toD3D12SrvDimension(rhi::TextureViewDimension dimension);
    static D3D12_UAV_DIMENSION toD3D12UavDimension(rhi::TextureViewDimension dimension);
    static D3D12_RTV_DIMENSION toD3D12RtvDimension(rhi::TextureViewDimension dimension);
    static D3D12_DSV_DIMENSION toD3D12DsvDimension(rhi::TextureViewDimension dimension);

private:
    Microsoft::WRL::ComPtr<IDXGIFactory6> m_Factory;
    Microsoft::WRL::ComPtr<IDXGIAdapter1> m_Adapter;
    Microsoft::WRL::ComPtr<ID3D12Device> m_Device;
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_SamplerHeap;
    uint32_t m_SamplerHeapOffset = 0;
    uint32_t m_SamplerDescriptorSize = 0;
    std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> m_Buffers;
    std::vector<rhi::BufferDesc> m_BufferMetadata;
    std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> m_Textures;
    std::vector<rhi::TextureDesc> m_TextureMetadata;
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_RtvHeap;
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_DsvHeap;
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_CbvSrvUavHeap;
    uint32_t m_RtvHeapOffset = 0, m_RtvDescriptorSize = 0;
    uint32_t m_DsvHeapOffset = 0, m_DsvDescriptorSize = 0;
    uint32_t m_CbvSrvUavHeapOffset = 0, m_CbvSrvUavDescriptorSize = 0;
    struct TextureViewData {
        D3D12_CPU_DESCRIPTOR_HANDLE srv{};
        D3D12_CPU_DESCRIPTOR_HANDLE uav{};
        D3D12_CPU_DESCRIPTOR_HANDLE rtv{};
        D3D12_CPU_DESCRIPTOR_HANDLE dsv{};
        bool hasSrv = false, hasUav = false, hasRtv = false, hasDsv = false;
    };
    std::vector<TextureViewData> m_TextureViews;
    D3D12_CPU_DESCRIPTOR_HANDLE AllocateCpuHandle(
        Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>& heap,
        uint32_t& offset,
        uint32_t descriptorSize);
};
