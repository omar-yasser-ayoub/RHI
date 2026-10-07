#include "rhi/vulkan/vkrhi.h"

void VulkanRHI::BeginRenderPass(const rhi::RenderPassDesc& desc)
{
}

void VulkanRHI::BeginComputePass()
{
}

void VulkanRHI::EndPass()
{
}

void VulkanRHI::SetPipeline(rhi::GraphicsPipelineHandle handle)
{
}

void VulkanRHI::SetPipeline(rhi::ComputePipelineHandle handle)
{
}

void VulkanRHI::SetBindGroup(std::uint32_t slot, rhi::BindGroupHandle handle)
{
}

void VulkanRHI::SetVertexBuffer(std::uint32_t slot, rhi::BufferHandle handle, std::size_t offset)
{
}

void VulkanRHI::SetIndexBuffer(rhi::BufferHandle handle, rhi::IndexFormat format, std::size_t offset)
{
}

void VulkanRHI::SetViewport(float x, float y, float width, float height)
{
}

void VulkanRHI::SetScissor(std::uint32_t x, std::uint32_t y, std::uint32_t width, std::uint32_t height)
{
}

void VulkanRHI::Draw(std::uint32_t vertexCount, std::uint32_t instanceCount,
                      std::uint32_t firstVertex, std::uint32_t firstInstance)
{
}

void VulkanRHI::DrawIndexed(std::uint32_t indexCount, std::uint32_t instanceCount,
                             std::uint32_t firstIndex, std::int32_t baseVertex,
                             std::uint32_t firstInstance)
{
}

void VulkanRHI::DrawIndirect(rhi::BufferHandle params, std::size_t offset,
                              std::uint32_t drawCount, std::uint32_t stride)
{
}

void VulkanRHI::DrawIndexedIndirect(rhi::BufferHandle params, std::size_t offset,
                                     std::uint32_t drawCount, std::uint32_t stride)
{
}

void VulkanRHI::Dispatch(std::uint32_t x, std::uint32_t y, std::uint32_t z)
{
}

void VulkanRHI::DispatchIndirect(rhi::BufferHandle params, std::size_t offset)
{
}

void VulkanRHI::CopyBufferToBuffer(rhi::BufferHandle src, rhi::BufferHandle dst, std::size_t size,
                                    std::size_t srcOffset, std::size_t dstOffset)
{
}

void VulkanRHI::CopyBufferToTexture(rhi::BufferHandle src, rhi::TextureHandle dst,
                                     const rhi::TextureWriteDesc& region)
{
}

void VulkanRHI::CopyTextureToBuffer(rhi::TextureHandle src, rhi::BufferHandle dst,
                                     const rhi::TextureWriteDesc& region)
{
}

void VulkanRHI::Submit()
{
}
