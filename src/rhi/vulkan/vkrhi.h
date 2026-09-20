#pragma once

#include "rhi/resources/capabilities.h"
#include "rhi/resources/buffer.h"
#include "rhi/resources/texture.h"
#include <rhi/resources/shader.h>
#include <rhi/resources/bindgroup.h>
#include <rhi/resources/pipeline.h>
#include <rhi/resources/swapchain.h>
#include <rhi/resources/surface.h>
#include <rhi/resources/window.h>

#include <vulkan/vulkan.h>
#include <vector>
#include <Windows.h>
#include <third_party/dxc/inc/dxcapi.h>
#include <third_party/vma/vk_mem_alloc.h>
#include <wrl/client.h>

struct SpvReflectShaderModule;


class VulkanRHI
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
    rhi::ShaderReflection ParseReflection(
        SpvReflectShaderModule* reflection,
        rhi::ShaderStageFlags stage);

    VkFilter toVk(rhi::Filter mode);
    VkSamplerMipmapMode toVk(rhi::MipmapFilter mode);
    VkSamplerAddressMode toVk(rhi::AddressMode mode);
    VkCompareOp toVk(rhi::CompareOp mode);
    VkBorderColor toVk(rhi::BorderColor mode);
    VkBufferUsageFlags toVk(rhi::BufferUsage usage);
    VkMemoryPropertyFlags toVk(rhi::MemoryUsage usage);
    VkImageViewType toVk(rhi::TextureViewDimension type, int layerCount);
    VkFormat toVk(rhi::TextureFormat format);
    VkImageAspectFlags toVk(rhi::TextureAspect aspect);
    VkImageType toVk(rhi::TextureDimension type);
    VkSampleCountFlagBits toVk(rhi::SampleCount count);
    VkImageUsageFlags toVk(rhi::TextureUsage usage);
    VkFormat toVk(rhi::VertexFormat format);
    VkPrimitiveTopology toVk(rhi::PrimitiveTopology topology);
    VkCullModeFlags toVk(rhi::CullMode mode);
    VkFrontFace toVk(rhi::FrontFace face);
    VkPolygonMode toVk(rhi::FillMode mode);
    VkBlendFactor toVk(rhi::BlendFactor factor);
    VkBlendOp toVk(rhi::BlendOp op);
    VkColorComponentFlags toVk(rhi::ColorWriteMask mask);
    VmaAllocationCreateInfo ToVmaAllocInfo(rhi::MemoryUsage memoryUsage);

    rhi::Capabilities m_Capabilities;
    VkInstance m_Instance = VK_NULL_HANDLE;
    VkPhysicalDevice m_PhysicalDevice = VK_NULL_HANDLE;
    VkDevice m_Device = VK_NULL_HANDLE;
    VkQueue m_GraphicsQueue = VK_NULL_HANDLE;
    VmaAllocator m_Allocator = nullptr;
    VkDescriptorPool m_DescriptorPool = VK_NULL_HANDLE;
    std::vector<VkSampler> m_Samplers;
    std::vector<std::uint64_t> m_FreeSamplers;
    std::vector<VkBuffer> m_Buffers;
    std::vector<VmaAllocation> m_BufferMemory;
    std::vector<std::uint64_t> m_FreeBuffers;
    std::vector<VkImage> m_Textures;
    std::vector<VmaAllocation> m_TextureMemory;
    std::vector<std::uint64_t> m_FreeTextures;
    std::vector<VkImageView> m_TextureViews;
    std::vector<std::uint64_t> m_FreeTextureViews;
    std::vector<rhi::ShaderModule> m_ShaderModules;
    std::vector<std::uint64_t> m_FreeShaderModules;
    std::vector<VkDescriptorSetLayout> m_BindGroupLayouts;
    std::vector<std::uint64_t> m_FreeBindGroupLayouts;
    std::vector<VkDescriptorSet> m_BindGroups;
    std::vector<std::uint64_t> m_FreeBindGroups;
    std::vector<VkPipelineLayout> m_PipelineLayouts;
    std::vector<std::uint64_t> m_FreePipelineLayouts;
    std::vector<VkPipeline> m_Pipelines;
    std::vector<std::uint64_t> m_FreePipelines;
    std::vector<VkSurfaceKHR> m_Surfaces;
    std::vector<std::uint64_t> m_FreeSurfaces;
    std::vector<struct GLFWwindow*> m_Windows;
    std::vector<std::uint64_t> m_FreeWindows;
    bool m_GlfwInitialized = false;
};
