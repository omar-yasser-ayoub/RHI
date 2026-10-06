#include "rhi/webgpu/wgpurhi.h"

void WebGPURHI::BeginRenderPass(const rhi::RenderPassDesc& desc)
{
}

void WebGPURHI::BeginComputePass()
{
}

void WebGPURHI::EndPass()
{
}

void WebGPURHI::SetGraphicsPipeline(rhi::GraphicsPipelineHandle handle)
{
}

void WebGPURHI::SetComputePipeline(rhi::ComputePipelineHandle handle)
{
}

void WebGPURHI::SetBindGroup(std::uint32_t slot, rhi::BindGroupHandle handle)
{
}

void WebGPURHI::SetVertexBuffer(std::uint32_t slot, rhi::BufferHandle handle, std::size_t offset)
{
}

void WebGPURHI::SetIndexBuffer(rhi::BufferHandle handle, rhi::IndexFormat format, std::size_t offset)
{
}

void WebGPURHI::SetViewport(float x, float y, float width, float height)
{
}

void WebGPURHI::SetScissor(std::uint32_t x, std::uint32_t y, std::uint32_t width, std::uint32_t height)
{
}

void WebGPURHI::Draw(std::uint32_t vertexCount, std::uint32_t instanceCount,
                      std::uint32_t firstVertex, std::uint32_t firstInstance)
{
}

void WebGPURHI::DrawIndexed(std::uint32_t indexCount, std::uint32_t instanceCount,
                             std::uint32_t firstIndex, std::int32_t baseVertex,
                             std::uint32_t firstInstance)
{
}

void WebGPURHI::DrawIndirect(rhi::BufferHandle params, std::size_t offset,
                              std::uint32_t drawCount, std::uint32_t stride)
{
}

void WebGPURHI::DrawIndexedIndirect(rhi::BufferHandle params, std::size_t offset,
                                     std::uint32_t drawCount, std::uint32_t stride)
{
}

void WebGPURHI::Dispatch(std::uint32_t x, std::uint32_t y, std::uint32_t z)
{
}

void WebGPURHI::DispatchIndirect(rhi::BufferHandle params, std::size_t offset)
{
}

void WebGPURHI::CopyBufferToBuffer(rhi::BufferHandle src, rhi::BufferHandle dst, std::size_t size,
                                    std::size_t srcOffset, std::size_t dstOffset)
{
}

void WebGPURHI::CopyBufferToTexture(rhi::BufferHandle src, rhi::TextureHandle dst,
                                     const rhi::TextureWriteDesc& region)
{
}

void WebGPURHI::CopyTextureToBuffer(rhi::TextureHandle src, rhi::BufferHandle dst,
                                     const rhi::TextureWriteDesc& region)
{
}

void WebGPURHI::Submit()
{
}
