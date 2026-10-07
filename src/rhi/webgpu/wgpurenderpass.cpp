#include "rhi/webgpu/wgpurhi.h"

#include <vector>

void WebGPURHI::BeginRenderPass(const rhi::RenderPassDesc& desc)
{
    if (m_CurrentPass != currentPass::None) {
        printf("Another pass is currently running. End it before beginning a new pass.");
        return;
    }
    if (!m_CommandEncoder) {
        m_CommandEncoder = m_Device.CreateCommandEncoder();
    }

    // Get the swapchain's current texture, if we're actually rendering into it
    wgpu::TextureView swapchainView;
    for (const auto& attachment : desc.colors) {
        if (!attachment.useSwapchain) {
            continue;
        }
        if (desc.swapchain.id == 0xFFFFFFFFFFFFFFFF ||
            desc.swapchain.id >= m_Swapchains.size()) {
            printf("BeginRenderPass swapchain handle is invalid\n");
            return;
        }
        WebGPUSwapchainData& swapchain = m_Swapchains[desc.swapchain.id];
        if (!swapchain.surface || !swapchain.currentView) {
            printf("BeginRenderPass needs AcquireNextImage first\n");
            return;
        }
        swapchainView = swapchain.currentView;
        break;
    }

    std::vector<wgpu::RenderPassColorAttachment> colors;
    colors.reserve(desc.colors.size());
    for (const auto& attachment : desc.colors) {
        wgpu::TextureView view;
        if (attachment.useSwapchain) {
            view = swapchainView;
        } else {
            if (attachment.view.id == 0xFFFFFFFFFFFFFFFF ||
                attachment.view.id >= m_TextureViews.size() ||
                !m_TextureViews[attachment.view.id]) {
                printf("BeginRenderPass color view handle is invalid\n");
                return;
            }
            view = m_TextureViews[attachment.view.id];
        }

        wgpu::RenderPassColorAttachment color{};
        color.view = view;
        color.loadOp = attachment.clear ? wgpu::LoadOp::Clear : wgpu::LoadOp::Load;
        color.storeOp = wgpu::StoreOp::Store;
        color.clearValue = {
            attachment.clearColor[0],
            attachment.clearColor[1],
            attachment.clearColor[2],
            attachment.clearColor[3],
        };
        colors.push_back(color);
    }
    if (colors.empty()) {
        printf("BeginRenderPass needs at least one color attachment\n");
        return;
    }

    wgpu::RenderPassDepthStencilAttachment depthAttachment{};
    bool hasDepth = false;
    if (desc.depth.enabled) {
        if (desc.depth.view.id == 0xFFFFFFFFFFFFFFFF ||
            desc.depth.view.id >= m_TextureViews.size() ||
            !m_TextureViews[desc.depth.view.id]) {
            printf("BeginRenderPass depth view handle is invalid\n");
            return;
        }
        depthAttachment.view = m_TextureViews[desc.depth.view.id];
        depthAttachment.depthLoadOp =
            desc.depth.clearDepth ? wgpu::LoadOp::Clear : wgpu::LoadOp::Load;
        depthAttachment.depthStoreOp = wgpu::StoreOp::Store;
        depthAttachment.depthClearValue = desc.depth.clearDepthValue;
        depthAttachment.stencilLoadOp =
            desc.depth.clearStencil ? wgpu::LoadOp::Clear : wgpu::LoadOp::Load;
        depthAttachment.stencilStoreOp = wgpu::StoreOp::Store;
        depthAttachment.stencilClearValue = desc.depth.clearStencilValue;
        hasDepth = true;
    }

    wgpu::RenderPassDescriptor pass{};
    pass.colorAttachmentCount = colors.size();
    pass.colorAttachments = colors.data();
    if (hasDepth) {
        pass.depthStencilAttachment = &depthAttachment;
    }
    wgpu::RenderPassEncoder renderPass = m_CommandEncoder.BeginRenderPass(&pass);
    m_RenderPassEncoder = renderPass;

    m_CurrentPass = currentPass::Graphics;
}

void WebGPURHI::BeginComputePass()
{
    if (m_CurrentPass != currentPass::None) {
        printf("Another pass is currently running. End it before beginning a new pass.");
        return;
    }
    if (!m_CommandEncoder) {
        m_CommandEncoder = m_Device.CreateCommandEncoder();
    }
    wgpu::ComputePassDescriptor pass{};
    wgpu::ComputePassEncoder computePass = m_CommandEncoder.BeginComputePass(&pass);
    m_ComputePassEncoder = computePass;

    m_CurrentPass = currentPass::Compute;
}

void WebGPURHI::EndPass()
{
    if (m_ComputePassEncoder) {
        m_ComputePassEncoder.End();
        m_ComputePassEncoder = nullptr;
        m_CurrentPass = currentPass::None;
    }
    if (m_RenderPassEncoder) {
        m_RenderPassEncoder.End();
        m_RenderPassEncoder = nullptr;
        m_CurrentPass = currentPass::None;
    }
}

void WebGPURHI::SetPipeline(rhi::GraphicsPipelineHandle handle)
{
    if (m_CurrentPass != currentPass::Graphics) {
        printf("Currently not in graphics pass, cannot set graphics pipeline\n");
        return;
    }
    if (handle.id >= m_Pipelines.size() || !m_Pipelines[handle.id]) {
        printf("Invalid pipeline handle\n");
        return;
    }
    if (!m_RenderPassEncoder) {
        printf("No render pass encoder\n");
        return;
    }
    wgpu::RenderPipeline pipeline = m_Pipelines[handle.id];
    m_RenderPassEncoder.SetPipeline(pipeline);
}

void WebGPURHI::SetPipeline(rhi::ComputePipelineHandle handle)
{
    if (m_CurrentPass != currentPass::Compute) {
        printf("Currently not in compute pass, cannot set compute pipeline\n");
        return;
    }
    if (handle.id >= m_ComputePipelines.size() || !m_ComputePipelines[handle.id]) {
        printf("Invalid pipeline handle\n");
        return;
    }
    if (!m_ComputePassEncoder) {
        printf("No compute pass encoder\n");
        return;
    }
    wgpu::ComputePipeline pipeline = m_ComputePipelines[handle.id];
    m_ComputePassEncoder.SetPipeline(pipeline);
}

void WebGPURHI::SetBindGroup(std::uint32_t slot, rhi::BindGroupHandle handle)
{
    if (handle.id >= m_BindGroups.size() || !m_BindGroups[handle.id]) {
        printf("Invalid bind group handle\n");
        return;
    }
    if (m_CurrentPass == currentPass::None) {
        printf("No current pass\n");
        return;
    }
    if (m_CurrentPass == currentPass::Compute) {
        if (m_ComputePassEncoder) {
                wgpu::BindGroup bindGroup = m_BindGroups[handle.id];
                m_ComputePassEncoder.SetBindGroup(slot, bindGroup);
        }
    }
    else {
        if (m_RenderPassEncoder) {
            wgpu::BindGroup bindGroup = m_BindGroups[handle.id];
            m_RenderPassEncoder.SetBindGroup(slot, bindGroup);
        }
    }

}

void WebGPURHI::SetVertexBuffer(std::uint32_t slot, rhi::BufferHandle handle, std::size_t offset)
{
    if (m_CurrentPass != currentPass::Graphics) {
        printf("Currently not in graphics pass, cannot set vertex buffer\n");
        return;
    }
    if (handle.id >= m_Buffers.size() || !m_Buffers[handle.id]) {
        printf("Invalid buffer handle\n");
        return;
    }
    if (!m_RenderPassEncoder) {
        printf("No render pass encoder\n");
        return;
    }
    if (offset > handle.desc.size) {
        printf("SetVertexBuffer offset is out of range\n");
        return;
    }
    wgpu::Buffer buffer = m_Buffers[handle.id];
    m_RenderPassEncoder.SetVertexBuffer(slot, buffer, offset);
}

void WebGPURHI::SetIndexBuffer(rhi::BufferHandle handle, rhi::IndexFormat format, std::size_t offset)
{
    if (m_CurrentPass != currentPass::Graphics) {
        printf("Currently not in graphics pass, cannot set index buffer\n");
        return;
    }
    if (handle.id >= m_Buffers.size() || !m_Buffers[handle.id]) {
        printf("Invalid buffer handle\n");
        return;
    }
    if (!m_RenderPassEncoder) {
        printf("No render pass encoder\n");
        return;
    }
    if (offset > handle.desc.size) {
        printf("SetIndexBuffer offset is out of range\n");
        return;
    }
    wgpu::Buffer buffer = m_Buffers[handle.id];
    m_RenderPassEncoder.SetIndexBuffer(buffer, toWebGPU(format), offset);
}

void WebGPURHI::SetViewport(float x, float y, float width, float height)
{
    if (m_CurrentPass != currentPass::Graphics) {
        printf("Currently not in graphics pass, cannot set viewport\n");
        return;
    }
    if (!m_RenderPassEncoder) {
        printf("No render pass encoder\n");
        return;
    }
    m_RenderPassEncoder.SetViewport(x, y, width, height, 0.0f, 1.0f);
}

void WebGPURHI::SetScissor(std::uint32_t x, std::uint32_t y, std::uint32_t width, std::uint32_t height)
{
    if (m_CurrentPass != currentPass::Graphics) {
        printf("Currently not in graphics pass, cannot set scissor\n");
        return;
    }
    if (!m_RenderPassEncoder) {
        printf("No render pass encoder\n");
        return;
    }
    m_RenderPassEncoder.SetScissorRect(x, y, width, height);
}

void WebGPURHI::Draw(std::uint32_t vertexCount, std::uint32_t instanceCount,
                      std::uint32_t firstVertex, std::uint32_t firstInstance)
{
    if (m_CurrentPass != currentPass::Graphics) {
        printf("Currently not in graphics pass, cannot draw\n");
        return;
    }
    if (!m_RenderPassEncoder) {
        printf("No render pass encoder\n");
        return;
    }
    m_RenderPassEncoder.Draw(vertexCount, instanceCount, firstVertex, firstInstance);
}

void WebGPURHI::DrawIndexed(std::uint32_t indexCount, std::uint32_t instanceCount,
                             std::uint32_t firstIndex, std::int32_t baseVertex,
                             std::uint32_t firstInstance)
{
    if (m_CurrentPass != currentPass::Graphics) {
        printf("Currently not in graphics pass, cannot draw\n");
        return;
    }
    if (!m_RenderPassEncoder) {
        printf("No render pass encoder\n");
        return;
    }
    m_RenderPassEncoder.DrawIndexed(indexCount, instanceCount, firstIndex, baseVertex, firstInstance);
}

void WebGPURHI::DrawIndirect(rhi::BufferHandle params, std::size_t offset,
                              std::uint32_t drawCount, std::uint32_t stride)
{
    if (m_CurrentPass != currentPass::Graphics) {
        printf("Currently not in graphics pass, cannot draw\n");
        return;
    }
    if (params.id >= m_Buffers.size() || !m_Buffers[params.id]) {
        printf("Invalid params buffer handle\n");
        return;
    }
    if (offset > params.desc.size) {
        printf("DrawIndirect offset is out of range\n");
        return;
    }
    if (stride < 16) {
        printf("DrawIndirect stride must hold one draw call\n");
        return;
    }
    if (!m_RenderPassEncoder) {
        printf("No render pass encoder\n");
        return;
    }
    wgpu::Buffer buffer = m_Buffers[params.id];
    for (std::uint32_t i = 0; i < drawCount; ++i) {
        m_RenderPassEncoder.DrawIndirect(buffer, offset + i * stride);
    }
}

void WebGPURHI::DrawIndexedIndirect(rhi::BufferHandle params, std::size_t offset,
                                     std::uint32_t drawCount, std::uint32_t stride)
{
    if (m_CurrentPass != currentPass::Graphics) {
        printf("Currently not in graphics pass, cannot draw\n");
        return;
    }
    if (params.id >= m_Buffers.size() || !m_Buffers[params.id]) {
        printf("Invalid params buffer handle\n");
        return;
    }
    if (offset > params.desc.size) {
        printf("DrawIndexedIndirect offset is out of range\n");
        return;
    }
    if (stride < 20) {
        printf("DrawIndexedIndirect stride must hold one indexed draw call\n");
        return;
    }
    if (!m_RenderPassEncoder) {
        printf("No render pass encoder\n");
        return;
    }
    wgpu::Buffer buffer = m_Buffers[params.id];
    for (std::uint32_t i = 0; i < drawCount; ++i) {
        m_RenderPassEncoder.DrawIndexedIndirect(buffer, offset + i * stride);
    }
}

void WebGPURHI::Dispatch(std::uint32_t x, std::uint32_t y, std::uint32_t z)
{
    if (m_CurrentPass != currentPass::Compute) {
        printf("Currently not in compute pass, cannot dispatch\n");
        return;
    }
    if (!m_ComputePassEncoder) {
        printf("No compute pass encoder\n");
        return;
    }
    m_ComputePassEncoder.DispatchWorkgroups(x, y, z);
}

void WebGPURHI::DispatchIndirect(rhi::BufferHandle params, std::size_t offset)
{
    if (m_CurrentPass != currentPass::Compute) {
        printf("Currently not in compute pass, cannot dispatch\n");
        return;
    }
    if (params.id >= m_Buffers.size() || !m_Buffers[params.id]) {
        printf("Invalid params buffer handle\n");
        return;
    }
    if (offset > params.desc.size) {
        printf("DispatchIndirect offset is out of range\n");
        return;
    }
    if (!m_ComputePassEncoder) {
        printf("No compute pass encoder\n");
        return;
    }
    wgpu::Buffer buffer = m_Buffers[params.id];
    m_ComputePassEncoder.DispatchWorkgroupsIndirect(buffer, offset);
}

void WebGPURHI::CopyBufferToBuffer(rhi::BufferHandle src, rhi::BufferHandle dst, std::size_t size,
                                    std::size_t srcOffset, std::size_t dstOffset)
{
    if (src.id >= m_Buffers.size() || !m_Buffers[src.id] ||
        dst.id >= m_Buffers.size() || !m_Buffers[dst.id]) {
        printf("Invalid buffer handle\n");
        return;
    }
    if (size == 0 || srcOffset > src.desc.size || size > src.desc.size - srcOffset ||
        dstOffset > dst.desc.size || size > dst.desc.size - dstOffset) {
        printf("CopyBufferToBuffer range is out of bounds\n");
        return;
    }
    if (!m_CommandEncoder) {
        m_CommandEncoder = m_Device.CreateCommandEncoder();
    }
    m_CommandEncoder.CopyBufferToBuffer(
        m_Buffers[src.id], srcOffset, m_Buffers[dst.id], dstOffset, size);
}

void WebGPURHI::CopyBufferToTexture(rhi::BufferHandle src, rhi::TextureHandle dst,
                                     const rhi::TextureWriteDesc& region)
{
    if (src.id >= m_Buffers.size() || !m_Buffers[src.id] ||
        dst.id >= m_Textures.size() || !m_Textures[dst.id]) {
        printf("Invalid copy handle\n");
        return;
    }
    if (region.bytesPerRow % 256 != 0) {
        printf("CopyBufferToTexture row pitch must be 256-byte aligned\n");
        return;
    }
    const std::size_t needed =
        static_cast<std::size_t>(region.bytesPerRow) * region.rowsPerImage;
    if (needed > src.desc.size) {
        printf("CopyBufferToTexture source buffer is too small\n");
        return;
    }
    if (!m_CommandEncoder) {
        m_CommandEncoder = m_Device.CreateCommandEncoder();
    }

    wgpu::TexelCopyBufferInfo source{};
    source.buffer = m_Buffers[src.id];
    source.layout.offset = 0;
    source.layout.bytesPerRow = region.bytesPerRow;
    source.layout.rowsPerImage = region.rowsPerImage;

    wgpu::TexelCopyTextureInfo destination{};
    destination.texture = m_Textures[dst.id];
    destination.mipLevel = region.mipLevel;
    destination.origin = { region.x, region.y, region.z };
    destination.aspect = wgpu::TextureAspect::All;

    wgpu::Extent3D extent{ region.width, region.height, region.depth };
    m_CommandEncoder.CopyBufferToTexture(&source, &destination, &extent);
}

void WebGPURHI::CopyTextureToBuffer(rhi::TextureHandle src, rhi::BufferHandle dst,
                                     const rhi::TextureWriteDesc& region)
{
    if (src.id >= m_Textures.size() || !m_Textures[src.id] ||
        dst.id >= m_Buffers.size() || !m_Buffers[dst.id]) {
        printf("Invalid copy handle\n");
        return;
    }
    if (region.bytesPerRow % 256 != 0) {
        printf("CopyTextureToBuffer row pitch must be 256-byte aligned\n");
        return;
    }
    const std::size_t needed =
        static_cast<std::size_t>(region.bytesPerRow) * region.rowsPerImage;
    if (needed > dst.desc.size) {
        printf("CopyTextureToBuffer destination buffer is too small\n");
        return;
    }
    if (!m_CommandEncoder) {
        m_CommandEncoder = m_Device.CreateCommandEncoder();
    }

    wgpu::TexelCopyTextureInfo source{};
    source.texture = m_Textures[src.id];
    source.mipLevel = region.mipLevel;
    source.origin = { region.x, region.y, region.z };
    source.aspect = wgpu::TextureAspect::All;

    wgpu::TexelCopyBufferInfo destination{};
    destination.buffer = m_Buffers[dst.id];
    destination.layout.offset = 0;
    destination.layout.bytesPerRow = region.bytesPerRow;
    destination.layout.rowsPerImage = region.rowsPerImage;

    wgpu::Extent3D extent{ region.width, region.height, region.depth };
    m_CommandEncoder.CopyTextureToBuffer(&source, &destination, &extent);
}

void WebGPURHI::Submit()
{
    if (m_CurrentPass != currentPass::None) {
        printf("Submit needs EndPass first\n");
        return;
    }
    if (m_CommandEncoder) {
        wgpu::CommandBuffer commandBuffer = m_CommandEncoder.Finish();
        m_Queue.Submit(1, &commandBuffer);
        m_CommandEncoder = nullptr;
    }
}
