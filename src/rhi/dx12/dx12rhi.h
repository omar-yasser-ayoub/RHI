#pragma once

#include "rhi/irhi.h"

#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl/client.h>

class DX12RHI : public IRHI
{
public:
    bool Initialize();
    void Shutdown();

    rhi::SamplerHandle CreateSampler(const rhi::SamplerDesc& desc);
    rhi::TextureHandle CreateTexture(const rhi::TextureDesc& desc);
    rhi::BufferHandle CreateBuffer(const rhi::BufferDesc& desc, const void* initialData = nullptr);

    void DestroySampler(rhi::SamplerHandle handle);
    void DestroyTexture(rhi::TextureHandle handle);
    void DestroyBuffer(rhi::BufferHandle handle);

    void WriteBuffer(rhi::BufferHandle handle, const void* data, std::size_t size, std::size_t offset = 0);
    void WriteTexture(rhi::TextureHandle handle, const rhi::TextureWriteDesc& desc, const void* data);

    static D3D12_FILTER toD3D12(rhi::Filter min, rhi::Filter mag, rhi::MipmapFilter mip, bool anisotropic, bool compare);
    static D3D12_TEXTURE_ADDRESS_MODE toD3D12(rhi::AddressMode mode);
    static D3D12_COMPARISON_FUNC toD3D12(rhi::CompareOp mode);
    static std::array<float, 4> toD3D12(rhi::BorderColor color);

private:
    Microsoft::WRL::ComPtr<IDXGIFactory6> m_Factory;
    Microsoft::WRL::ComPtr<IDXGIAdapter1> m_Adapter;
    Microsoft::WRL::ComPtr<ID3D12Device> m_Device;
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_SamplerHeap;
    uint32_t m_SamplerHeapOffset = 0;
    uint32_t m_SamplerDescriptorSize = 0;
};
