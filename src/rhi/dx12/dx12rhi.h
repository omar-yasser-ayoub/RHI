#pragma once

#include "../IRHI.h"

#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl/client.h>

class DX12RHI : public IRHI
{
public:
    bool Initialize() override;
    void Shutdown() override;

    rhi::SamplerHandle CreateSampler(const rhi::SamplerDesc& desc) override;
    rhi::TextureHandle CreateTexture(const rhi::TextureDesc& desc) override;
    rhi::BufferHandle CreateBuffer(const rhi::BufferDesc& desc, const void* initialData = nullptr) override;

    void DestroySampler(rhi::SamplerHandle handle) override;
    void DestroyTexture(rhi::TextureHandle handle) override;
    void DestroyBuffer(rhi::BufferHandle handle) override;

    void WriteBuffer(rhi::BufferHandle handle, const void* data, std::size_t size, std::size_t offset = 0) override;
    void WriteTexture(rhi::TextureHandle handle, const rhi::TextureWriteDesc& desc, const void* data) override;


private:
    Microsoft::WRL::ComPtr<IDXGIFactory6> m_Factory;
    Microsoft::WRL::ComPtr<IDXGIAdapter1> m_Adapter;
    Microsoft::WRL::ComPtr<ID3D12Device> m_Device;
};
