#pragma once

#include "rhi/resources/capabilities.h"
#include "rhi/resources/buffer.h"
#include "rhi/resources/shader.h"
#include "rhi/resources/texture.h"
#include "rhi/resources/bindgroup.h"
#include "rhi/resources/pipeline.h"
#include "rhi/resources/swapchain.h"
#include "rhi/resources/surface.h"
#include "rhi/resources/window.h"
#include <webgpu/webgpu_cpp.h>

struct SpvReflectShaderModule;

class WebGPURHI
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
    rhi::TextureViewHandle CreateTextureView(const rhi::TextureViewDesc& desc, rhi::TextureHandle texture);
    rhi::ShaderModuleHandle CompileShader(rhi::ShaderSource source);
    const rhi::ShaderModule* GetShaderModule(rhi::ShaderModuleHandle handle) const;
    rhi::BindGroupLayoutHandle CreateBindGroupLayout(const rhi::BindGroupLayoutDesc& desc);
    rhi::BindGroupHandle CreateBindGroup(const rhi::BindGroupDesc& desc);
    rhi::PipelineLayoutHandle CreatePipelineLayout(const rhi::PipelineLayoutDesc& desc);
    rhi::GraphicsPipelineHandle CreateGraphicsPipeline(const rhi::GraphicsPipelineDesc& desc);
    rhi::WindowHandle CreateNativeWindow(const rhi::WindowDesc& desc);
    rhi::SurfaceHandle CreateSurface(const rhi::SurfaceDesc& desc);
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
    void DestroySurface(rhi::SurfaceHandle handle);
    void DestroySwapchain(rhi::SwapchainHandle handle);

    void WriteBuffer(rhi::BufferHandle handle, const void* data, std::size_t size, std::size_t offset = 0);

    void* MapBuffer(rhi::BufferHandle handle);
    void UnmapBuffer(rhi::BufferHandle handle);

private:
    static wgpu::FilterMode toWebGPU(rhi::Filter f);
    static wgpu::MipmapFilterMode toWebGPU(rhi::MipmapFilter f);
    static wgpu::AddressMode toWebGPU(rhi::AddressMode f);
    static wgpu::CompareFunction toWebGPU(rhi::CompareOp op);
    static wgpu::BufferUsage toWebGPU(rhi::BufferUsage usage);
    static wgpu::BufferUsage toWebGPU(rhi::MemoryUsage usage);
    static wgpu::TextureDimension toWebGPU(rhi::TextureDimension dim);
    static wgpu::TextureFormat toWebGPU(rhi::TextureFormat usage);
    static uint32_t toWebGPU(rhi::SampleCount count);
    static wgpu::TextureUsage toWebGPU(rhi::TextureUsage usage);
    static wgpu::VertexFormat toWebGPU(rhi::VertexFormat format);
    static wgpu::PrimitiveTopology toWebGPU(rhi::PrimitiveTopology topology);
    static wgpu::CullMode toWebGPU(rhi::CullMode mode);
    static wgpu::FrontFace toWebGPU(rhi::FrontFace face);
    static wgpu::BlendFactor toWebGPU(rhi::BlendFactor factor);
    static wgpu::BlendOperation toWebGPU(rhi::BlendOp op);
    static wgpu::ColorWriteMask toWebGPU(rhi::ColorWriteMask mask);
    static rhi::ShaderReflection ParseReflection(
        SpvReflectShaderModule* reflection,
        rhi::ShaderStageFlags stage);
    wgpu::ShaderModule MakeShaderModule(const rhi::ShaderModule& module);


    rhi::Capabilities m_Capabilities;
    wgpu::Instance m_Instance;
    wgpu::Adapter  m_Adapter;
    wgpu::Device   m_Device;
    wgpu::Queue    m_Queue;
    std::vector<wgpu::Sampler> m_Samplers;
    std::vector<std::uint64_t> m_FreeSamplers;
    std::vector<wgpu::Buffer> m_Buffers;
    std::vector<std::uint64_t> m_FreeBuffers;
    std::vector<wgpu::Texture> m_Textures;
    std::vector<std::uint64_t> m_FreeTextures;
    std::vector<wgpu::TextureView> m_TextureViews;
    std::vector<std::uint64_t> m_FreeTextureViews;
    std::vector<rhi::ShaderModule> m_ShaderModules;
    std::vector<std::uint64_t> m_FreeShaderModules;
    std::vector<wgpu::BindGroupLayout> m_BindGroupLayouts;
    std::vector<std::uint64_t> m_FreeBindGroupLayouts;
    std::vector<wgpu::BindGroup> m_BindGroups;
    std::vector<std::uint64_t> m_FreeBindGroups;
    std::vector<wgpu::PipelineLayout> m_PipelineLayouts;
    std::vector<std::uint64_t> m_FreePipelineLayouts;
    std::vector<wgpu::RenderPipeline> m_Pipelines;
    std::vector<std::uint64_t> m_FreePipelines;
    std::vector<wgpu::Surface> m_Surfaces;
    std::vector<std::uint64_t> m_FreeSurfaces;
    std::vector<struct GLFWwindow*> m_Windows;
    std::vector<std::uint64_t> m_FreeWindows;
    bool m_GlfwInitialized = false;
};
