#include "rhi/vulkan/vkrhi.h"

rhi::CommandListHandle VulkanRHI::BeginCommandList()
{
    return {};
}

void VulkanRHI::BeginRenderPass(rhi::CommandListHandle cmd, const rhi::RenderPassDesc& desc)
{
}

void VulkanRHI::BeginComputePass(rhi::CommandListHandle cmd)
{
}

void VulkanRHI::EndPass(rhi::CommandListHandle cmd)
{
}

void VulkanRHI::SetPipeline(rhi::CommandListHandle cmd, rhi::GraphicsPipelineHandle handle)
{
}

void VulkanRHI::SetPipeline(rhi::CommandListHandle cmd, rhi::ComputePipelineHandle handle)
{
}

void VulkanRHI::SetBindGroup(rhi::CommandListHandle cmd, std::uint32_t slot, rhi::BindGroupHandle handle)
{
}

void VulkanRHI::SetVertexBuffer(rhi::CommandListHandle cmd, std::uint32_t slot, rhi::BufferHandle handle, std::size_t offset)
{
}

void VulkanRHI::SetIndexBuffer(rhi::CommandListHandle cmd, rhi::BufferHandle handle, rhi::IndexFormat format, std::size_t offset)
{
}

void VulkanRHI::SetViewport(rhi::CommandListHandle cmd, float x, float y, float width, float height)
{
}

void VulkanRHI::SetScissor(rhi::CommandListHandle cmd, std::uint32_t x, std::uint32_t y, std::uint32_t width, std::uint32_t height)
{
}

void VulkanRHI::Draw(rhi::CommandListHandle cmd, std::uint32_t vertexCount, std::uint32_t instanceCount,
                      std::uint32_t firstVertex, std::uint32_t firstInstance)
{
}

void VulkanRHI::DrawIndexed(rhi::CommandListHandle cmd, std::uint32_t indexCount, std::uint32_t instanceCount,
                             std::uint32_t firstIndex, std::int32_t baseVertex,
                             std::uint32_t firstInstance)
{
}

void VulkanRHI::DrawIndirect(rhi::CommandListHandle cmd, rhi::BufferHandle params, std::size_t offset,
                              std::uint32_t drawCount, std::uint32_t stride)
{
}

void VulkanRHI::DrawIndexedIndirect(rhi::CommandListHandle cmd, rhi::BufferHandle params, std::size_t offset,
                                     std::uint32_t drawCount, std::uint32_t stride)
{
}

void VulkanRHI::Dispatch(rhi::CommandListHandle cmd, std::uint32_t x, std::uint32_t y, std::uint32_t z)
{
}

void VulkanRHI::DispatchIndirect(rhi::CommandListHandle cmd, rhi::BufferHandle params, std::size_t offset)
{
}

void VulkanRHI::CopyBufferToBuffer(rhi::CommandListHandle cmd, rhi::BufferHandle src, rhi::BufferHandle dst, std::size_t size,
                                    std::size_t srcOffset, std::size_t dstOffset)
{
}

void VulkanRHI::CopyBufferToTexture(rhi::CommandListHandle cmd, rhi::BufferHandle src, rhi::TextureHandle dst,
                                     const rhi::TextureWriteDesc& region)
{
}

void VulkanRHI::CopyTextureToBuffer(rhi::CommandListHandle cmd, rhi::TextureHandle src, rhi::BufferHandle dst,
                                     const rhi::TextureWriteDesc& region)
{
}

void VulkanRHI::Submit(std::span<const rhi::CommandListHandle> cmds)
{
}
