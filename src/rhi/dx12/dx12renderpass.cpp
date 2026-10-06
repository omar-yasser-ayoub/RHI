#include "rhi/dx12/dx12rhi.h"

void DX12RHI::BeginRenderPass(const rhi::RenderPassDesc& desc)
{
}

void DX12RHI::BeginComputePass()
{
}

void DX12RHI::EndPass()
{
}

void DX12RHI::SetGraphicsPipeline(rhi::GraphicsPipelineHandle handle)
{
}

void DX12RHI::SetComputePipeline(rhi::ComputePipelineHandle handle)
{
}

void DX12RHI::SetBindGroup(std::uint32_t slot, rhi::BindGroupHandle handle)
{
}

void DX12RHI::SetVertexBuffer(std::uint32_t slot, rhi::BufferHandle handle, std::size_t offset)
{
}

void DX12RHI::SetIndexBuffer(rhi::BufferHandle handle, rhi::IndexFormat format, std::size_t offset)
{
}

void DX12RHI::SetViewport(float x, float y, float width, float height)
{
}

void DX12RHI::SetScissor(std::uint32_t x, std::uint32_t y, std::uint32_t width, std::uint32_t height)
{
}

void DX12RHI::Draw(std::uint32_t vertexCount, std::uint32_t instanceCount,
                    std::uint32_t firstVertex, std::uint32_t firstInstance)
{
}

void DX12RHI::DrawIndexed(std::uint32_t indexCount, std::uint32_t instanceCount,
                           std::uint32_t firstIndex, std::int32_t baseVertex,
                           std::uint32_t firstInstance)
{
}

void DX12RHI::DrawIndirect(rhi::BufferHandle params, std::size_t offset,
                            std::uint32_t drawCount, std::uint32_t stride)
{
}

void DX12RHI::DrawIndexedIndirect(rhi::BufferHandle params, std::size_t offset,
                                   std::uint32_t drawCount, std::uint32_t stride)
{
}

void DX12RHI::Dispatch(std::uint32_t x, std::uint32_t y, std::uint32_t z)
{
}

void DX12RHI::DispatchIndirect(rhi::BufferHandle params, std::size_t offset)
{
}

void DX12RHI::CopyBufferToBuffer(rhi::BufferHandle src, rhi::BufferHandle dst, std::size_t size,
                                  std::size_t srcOffset, std::size_t dstOffset)
{
}

void DX12RHI::CopyBufferToTexture(rhi::BufferHandle src, rhi::TextureHandle dst,
                                   const rhi::TextureWriteDesc& region)
{
}

void DX12RHI::CopyTextureToBuffer(rhi::TextureHandle src, rhi::BufferHandle dst,
                                   const rhi::TextureWriteDesc& region)
{
}

void DX12RHI::Submit()
{
}
