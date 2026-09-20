#pragma once

#include "rhi/resources/capabilities.h"
#include "rhi/resources/texture.h"
#include "rhi/resources/shader.h"
#include "rhi/resources/bindgroup.h"
#include "rhi/resources/pipeline.h"
#include "rhi/resources/swapchain.h"
#include "rhi/resources/window.h"


#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl/client.h>
#include <vector>
#include <third_party/dxc/inc/dxcapi.h>

struct ID3D12ShaderReflection;

class DX12RHI
{
public:
    bool Initialize();
    void Shutdown();

    const rhi::Capabilities& GetCapabilities() const
    {
        return m_Capabilities;
    }

    rhi::SamplerHandle CreateSampler(const rhi::SamplerDesc& desc);
    rhi::TextureHandle CreateTexture(const rhi::TextureDesc& desc);
    rhi::BufferHandle CreateBuffer(const rhi::BufferDesc& desc);
    rhi::TextureViewHandle CreateTextureView(const rhi::TextureViewDesc& desc, rhi::TextureHandle handle);
    rhi::ShaderModuleHandle CompileShader(rhi::ShaderSource source);
    const rhi::ShaderModule* GetShaderModule(rhi::ShaderModuleHandle handle) const;
    rhi::BindGroupLayoutHandle CreateBindGroupLayout(const rhi::BindGroupLayoutDesc& desc);
    rhi::BindGroupHandle CreateBindGroup(const rhi::BindGroupDesc& desc);
    rhi::PipelineLayoutHandle CreatePipelineLayout(const rhi::PipelineLayoutDesc& desc);
    rhi::GraphicsPipelineHandle CreateGraphicsPipeline(const rhi::GraphicsPipelineDesc& desc);
    rhi::WindowHandle CreateNativeWindow(const rhi::WindowDesc& desc);
    rhi::SwapchainHandle CreateSwapchain(const rhi::SwapchainDesc& desc);

    rhi::AcquireResult AcquireNextImage(rhi::SwapchainHandle handle);
    void Present(rhi::SwapchainHandle handle);

    void DestroySampler(rhi::SamplerHandle handle);
    void DestroyTexture(rhi::TextureHandle handle);
    void DestroyBuffer(rhi::BufferHandle handle);
    void DestroyTextureView(rhi::TextureViewHandle handle);
    void DestroyBindGroupLayout(rhi::BindGroupLayoutHandle handle);
    void DestroyBindGroup(rhi::BindGroupHandle handle);
    void DestroyShaderModule(rhi::ShaderModuleHandle handle);
    void DestroyPipelineLayout(rhi::PipelineLayoutHandle handle);
    void DestroyPipeline(rhi::GraphicsPipelineHandle handle);
    void DestroyNativeWindow(rhi::WindowHandle handle);
    void DestroySwapchain(rhi::SwapchainHandle handle);


    void WriteBuffer(rhi::BufferHandle handle, const void* data, std::size_t size, std::size_t offset = 0);

    void* MapBuffer(rhi::BufferHandle handle);
    void UnmapBuffer(rhi::BufferHandle handle);

private:
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
    static DXGI_FORMAT toD3D12(rhi::VertexFormat format);
    static D3D12_PRIMITIVE_TOPOLOGY_TYPE toD3D12(rhi::PrimitiveTopology topology);
    static D3D12_CULL_MODE toD3D12(rhi::CullMode mode);
    static D3D12_FILL_MODE toD3D12(rhi::FillMode mode);
    static D3D12_BLEND toD3D12(rhi::BlendFactor factor);
    static D3D12_BLEND_OP toD3D12(rhi::BlendOp op);
    static UINT8 toD3D12(rhi::ColorWriteMask mask);
    static D3D12_DESCRIPTOR_RANGE_TYPE toD3D12(rhi::BindingType type);
    static D3D12_SHADER_VISIBILITY toD3D12(rhi::ShaderStageFlags stage);
    static D3D12_SHADER_VISIBILITY toD3D12(const rhi::BindGroupLayoutDesc& desc);
    static const rhi::BindGroupEntry* FindEntry(
        const rhi::BindGroupDesc& desc,
        std::uint32_t binding);
    static const rhi::ShaderBinding* FindBinding(
        const rhi::BindGroupLayoutDesc& desc,
        std::uint32_t binding);
    static rhi::ShaderReflection ParseReflection(
        ID3D12ShaderReflection* reflection,
        rhi::ShaderStageFlags stage);

    rhi::Capabilities m_Capabilities;
    Microsoft::WRL::ComPtr<IDXGIFactory6> m_Factory;
    Microsoft::WRL::ComPtr<IDXGIAdapter1> m_Adapter;
    Microsoft::WRL::ComPtr<ID3D12Device> m_Device;
    Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_CommandQueue;
    Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_ComputeQueue;
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_SamplerHeap;
    uint32_t m_SamplerHeapOffset = 0;
    uint32_t m_SamplerDescriptorSize = 0;
    struct SamplerViewData {
        D3D12_CPU_DESCRIPTOR_HANDLE cpu{};
        D3D12_GPU_DESCRIPTOR_HANDLE gpu{};
        bool hasSampler = false;
    };
    std::vector<SamplerViewData> m_SamplerViews;
    std::vector<std::uint64_t> m_FreeSamplers;
    std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> m_Buffers;
    struct BufferViewData {
        D3D12_CPU_DESCRIPTOR_HANDLE cbv{};
        D3D12_CPU_DESCRIPTOR_HANDLE srv{};
        D3D12_CPU_DESCRIPTOR_HANDLE uav{};

        D3D12_GPU_DESCRIPTOR_HANDLE gpuCbv{};
        D3D12_GPU_DESCRIPTOR_HANDLE gpuSrv{};
        D3D12_GPU_DESCRIPTOR_HANDLE gpuUav{};

        bool hasCbv = false;
        bool hasSrv = false;
        bool hasUav = false;
    };
    std::vector<BufferViewData> m_BufferViews;
    std::vector<std::uint64_t> m_FreeBuffers;
    std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> m_Textures;
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
        D3D12_GPU_DESCRIPTOR_HANDLE gpuSrv{};
        D3D12_GPU_DESCRIPTOR_HANDLE gpuUav{};
        D3D12_GPU_DESCRIPTOR_HANDLE gpuRtv{};
        D3D12_GPU_DESCRIPTOR_HANDLE gpuDsv{};
        bool hasSrv = false, hasUav = false, hasRtv = false, hasDsv = false;
    };
    std::vector<TextureViewData> m_TextureViews;
    std::vector<std::uint64_t> m_FreeTextures;
    std::vector<std::uint64_t> m_FreeTextureViews;
        struct DescriptorHandles
    {
        D3D12_CPU_DESCRIPTOR_HANDLE cpu{};
        D3D12_GPU_DESCRIPTOR_HANDLE gpu{};
        bool valid = false;
    };
    DX12RHI::DescriptorHandles AllocateDescriptor(
        Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>& heap,
        uint32_t& offset,
        uint32_t descriptorSize
    );
    struct BindGroupLayoutData
    {
        rhi::BindGroupLayoutDesc desc;
    };

    struct BindGroupData
    {
        rhi::BindGroupDesc desc;
        D3D12_GPU_DESCRIPTOR_HANDLE cbvSrvUavTable{};
        D3D12_GPU_DESCRIPTOR_HANDLE samplerTable{};
        uint32_t cbvSrvUavCount = 0;
        uint32_t samplerCount = 0;
    };

    std::vector<rhi::ShaderModule> m_ShaderModules;
    std::vector<std::uint64_t> m_FreeShaderModules;
    std::vector<BindGroupLayoutData> m_BindGroupLayouts;
    std::vector<std::uint64_t> m_FreeBindGroupLayouts;
    std::vector<BindGroupData> m_BindGroups;
    std::vector<std::uint64_t> m_FreeBindGroups;
    std::vector<Microsoft::WRL::ComPtr<ID3D12RootSignature>> m_PipelineLayouts;
    std::vector<std::uint64_t> m_FreePipelineLayouts;
    std::vector<Microsoft::WRL::ComPtr<ID3D12PipelineState>> m_Pipelines;
    std::vector<std::uint64_t> m_FreePipelines;
    std::vector<struct GLFWwindow*> m_Windows;
    std::vector<std::uint64_t> m_FreeWindows;
    bool m_GlfwInitialized = false;
};
