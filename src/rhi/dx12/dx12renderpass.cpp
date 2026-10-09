#include "rhi/dx12/dx12rhi.h"

// NOTE: command-list recording is not implemented yet. These stubs exist only
// to keep the backend compiling against the shared RHI API; every function is
// a no-op until the DX12 frame-context / allocator / barrier work lands.

void DX12RHI::BeginRenderPass(rhi::CommandListHandle, const rhi::RenderPassDesc&)
{
}

void DX12RHI::BeginComputePass(rhi::CommandListHandle)
{
}

void DX12RHI::EndPass(rhi::CommandListHandle)
{
}

void DX12RHI::SetPipeline(rhi::CommandListHandle, rhi::GraphicsPipelineHandle)
{
}

void DX12RHI::SetPipeline(rhi::CommandListHandle, rhi::ComputePipelineHandle)
{
}

void DX12RHI::SetBindGroup(rhi::CommandListHandle, std::uint32_t, rhi::BindGroupHandle)
{
}

void DX12RHI::SetVertexBuffer(rhi::CommandListHandle, std::uint32_t, rhi::BufferHandle, std::size_t)
{
}

void DX12RHI::SetIndexBuffer(rhi::CommandListHandle, rhi::BufferHandle, rhi::IndexFormat, std::size_t)
{
}

void DX12RHI::SetViewport(rhi::CommandListHandle, float, float, float, float)
{
}

void DX12RHI::SetScissor(rhi::CommandListHandle, std::uint32_t, std::uint32_t, std::uint32_t, std::uint32_t)
{
}

void DX12RHI::Draw(rhi::CommandListHandle, std::uint32_t, std::uint32_t,
                    std::uint32_t, std::uint32_t)
{
}

void DX12RHI::DrawIndexed(rhi::CommandListHandle, std::uint32_t, std::uint32_t,
                           std::uint32_t, std::int32_t,
                           std::uint32_t)
{
}

void DX12RHI::DrawIndirect(rhi::CommandListHandle, rhi::BufferHandle, std::size_t,
                            std::uint32_t, std::uint32_t)
{
}

void DX12RHI::DrawIndexedIndirect(rhi::CommandListHandle, rhi::BufferHandle, std::size_t,
                                   std::uint32_t, std::uint32_t)
{
}

void DX12RHI::Dispatch(rhi::CommandListHandle, std::uint32_t, std::uint32_t, std::uint32_t)
{
}

void DX12RHI::DispatchIndirect(rhi::CommandListHandle, rhi::BufferHandle, std::size_t)
{
}

void DX12RHI::CopyBufferToBuffer(rhi::CommandListHandle, rhi::BufferHandle, rhi::BufferHandle, std::size_t,
                                  std::size_t, std::size_t)
{
}

void DX12RHI::CopyBufferToTexture(rhi::CommandListHandle, rhi::BufferHandle, rhi::TextureHandle,
                                   const rhi::TextureWriteDesc&)
{
}

void DX12RHI::CopyTextureToBuffer(rhi::CommandListHandle, rhi::TextureHandle, rhi::BufferHandle,
                                   const rhi::TextureWriteDesc&)
{
}

void DX12RHI::CopyTextureToTexture(rhi::CommandListHandle, rhi::TextureHandle, rhi::TextureHandle,
                                    const rhi::TextureCopyRegion&)
{
}

rhi::CommandListHandle DX12RHI::BeginCommandList()
{
    return rhi::CommandListHandle{};
}

void DX12RHI::Submit(std::span<const rhi::CommandListHandle>)
{
}
