#include "rhi/vulkan/vkrhi.h"

#include <cstdio>
#include <vector>

rhi::CommandListHandle VulkanRHI::BeginCommandList()
{
    rhi::CommandListHandle handle;
    if (m_Device == VK_NULL_HANDLE || m_CurrentFrame == 0xFFFFFFFF) {
        return handle;
    }
    FrameContext& frame = m_Frames[m_CurrentFrame];

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = frame.pool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer buffer = VK_NULL_HANDLE;
    if (vkAllocateCommandBuffers(m_Device, &allocInfo, &buffer) != VK_SUCCESS) {
        return handle;
    }

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    if (vkBeginCommandBuffer(buffer, &beginInfo) != VK_SUCCESS) {
        return handle;
    }

    VulkanCommandList list;
    list.frame = m_CurrentFrame;
    list.bufferIndex = static_cast<std::uint32_t>(frame.cmds.size());
    list.open = true;
    frame.cmds.push_back(buffer);

    if (!m_FreeCommandLists.empty()) {
        handle.id = m_FreeCommandLists.back();
        m_FreeCommandLists.pop_back();
        m_CommandLists[handle.id] = list;
    } else {
        handle.id = static_cast<std::uint64_t>(m_CommandLists.size());
        m_CommandLists.push_back(list);
    }
    return handle;
}

VkCommandBuffer VulkanRHI::GetVkCommandBuffer(rhi::CommandListHandle handle)
{
    if (handle.id >= m_CommandLists.size()) {
        return VK_NULL_HANDLE;
    }
    VulkanCommandList& list = m_CommandLists[handle.id];
    if (!list.open || list.frame >= 2 ||
        list.bufferIndex >= m_Frames[list.frame].cmds.size()) {
        return VK_NULL_HANDLE;
    }
    return m_Frames[list.frame].cmds[list.bufferIndex];
}

static void TransitionImage(VkCommandBuffer buffer, VkImage image,
                            VkImageLayout oldLayout, VkImageLayout newLayout,
                            VkImageAspectFlags aspect)
{
    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    barrier.subresourceRange.aspectMask = aspect;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;
    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED) {
        barrier.srcAccessMask = 0;
    } else if (oldLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) {
        barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    } else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    } else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL) {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
    }
    if (newLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) {
        barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    } else if (newLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR) {
        barrier.dstAccessMask = 0;
    } else if (newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    } else if (newLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL) {
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
    }
    vkCmdPipelineBarrier(buffer,
                         VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                         VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                         0, 0, nullptr, 0, nullptr, 1, &barrier);
}

void VulkanRHI::BeginRenderPass(rhi::CommandListHandle cmd, const rhi::RenderPassDesc& desc)
{
    VkCommandBuffer buffer = GetVkCommandBuffer(cmd);
    if (buffer == VK_NULL_HANDLE) {
        return;
    }
    VulkanCommandList& list = m_CommandLists[cmd.id];
    if (list.pass != CurrentPass::None) {
        printf("BeginRenderPass: a pass is already open on this command list\n");
        return;
    }
    if (desc.colors.empty()) {
        printf("BeginRenderPass needs at least one color attachment\n");
        return;
    }

    std::vector<VkRenderingAttachmentInfo> colors;
    colors.reserve(desc.colors.size());
    VkExtent2D extent{};
    bool extentSet = false;

    for (const auto& attachment : desc.colors) {
        VkImageView view = VK_NULL_HANDLE;
        if (attachment.useSwapchain) {
            if (desc.swapchain.id == 0xFFFFFFFFFFFFFFFF ||
                desc.swapchain.id >= m_Swapchains.size()) {
                printf("BeginRenderPass swapchain handle is invalid\n");
                return;
            }
            VkSwapchainData& swapchain = m_Swapchains[desc.swapchain.id];
            if (desc.imageIndex >= swapchain.views.size()) {
                printf("BeginRenderPass swapchain image index is invalid\n");
                return;
            }
            view = swapchain.views[desc.imageIndex];
            VkImage image = swapchain.images[desc.imageIndex];
            TransitionImage(buffer, image,
                            VK_IMAGE_LAYOUT_UNDEFINED,
                            VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                            VK_IMAGE_ASPECT_COLOR_BIT);
            list.swapchainImage = image;
            extent = swapchain.extent;
            extentSet = true;
        } else {
            if (attachment.view.id == 0xFFFFFFFFFFFFFFFF ||
                attachment.view.id >= m_TextureViews.size() ||
                m_TextureViews[attachment.view.id] == VK_NULL_HANDLE) {
                printf("BeginRenderPass color view handle is invalid\n");
                return;
            }
            view = m_TextureViews[attachment.view.id];
        }

        VkRenderingAttachmentInfo color{};
        color.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
        color.imageView = view;
        color.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        color.loadOp = attachment.clear ? VK_ATTACHMENT_LOAD_OP_CLEAR
                                        : VK_ATTACHMENT_LOAD_OP_LOAD;
        color.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        color.clearValue.color.float32[0] = attachment.clearColor[0];
        color.clearValue.color.float32[1] = attachment.clearColor[1];
        color.clearValue.color.float32[2] = attachment.clearColor[2];
        color.clearValue.color.float32[3] = attachment.clearColor[3];
        colors.push_back(color);
    }

    VkRenderingAttachmentInfo* depthInfo = nullptr;
    VkRenderingAttachmentInfo depthStorage{};
    if (desc.depth.enabled) {
        if (desc.depth.view.id == 0xFFFFFFFFFFFFFFFF ||
            desc.depth.view.id >= m_TextureViews.size() ||
            m_TextureViews[desc.depth.view.id] == VK_NULL_HANDLE) {
            printf("BeginRenderPass depth view handle is invalid\n");
            return;
        }
        depthStorage.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
        depthStorage.imageView = m_TextureViews[desc.depth.view.id];
        depthStorage.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        depthStorage.loadOp = desc.depth.clearDepth ? VK_ATTACHMENT_LOAD_OP_CLEAR
                                                    : VK_ATTACHMENT_LOAD_OP_LOAD;
        depthStorage.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        depthStorage.clearValue.depthStencil.depth = desc.depth.clearDepthValue;
        depthStorage.clearValue.depthStencil.stencil = desc.depth.clearStencilValue;
        depthInfo = &depthStorage;
    }

    VkRenderingInfo renderInfo{};
    renderInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
    renderInfo.renderArea.offset = { 0, 0 };
    renderInfo.renderArea.extent = extentSet ? extent : VkExtent2D{ 1, 1 };
    renderInfo.layerCount = 1;
    renderInfo.colorAttachmentCount = static_cast<std::uint32_t>(colors.size());
    renderInfo.pColorAttachments = colors.data();
    renderInfo.pDepthAttachment = depthInfo;

    vkCmdBeginRendering(buffer, &renderInfo);
    list.pass = CurrentPass::Graphics;
    list.bindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
}

void VulkanRHI::BeginComputePass(rhi::CommandListHandle cmd)
{
    VkCommandBuffer buffer = GetVkCommandBuffer(cmd);
    if (buffer == VK_NULL_HANDLE) {
        return;
    }
    VulkanCommandList& list = m_CommandLists[cmd.id];
    if (list.pass != CurrentPass::None) {
        printf("BeginComputePass: a pass is already open on this command list\n");
        return;
    }
    list.pass = CurrentPass::Compute;
    list.bindPoint = VK_PIPELINE_BIND_POINT_COMPUTE;
}

void VulkanRHI::EndPass(rhi::CommandListHandle cmd)
{
    VkCommandBuffer buffer = GetVkCommandBuffer(cmd);
    if (buffer == VK_NULL_HANDLE) {
        return;
    }
    VulkanCommandList& list = m_CommandLists[cmd.id];
    if (list.pass == CurrentPass::Graphics) {
        vkCmdEndRendering(buffer);
        if (list.swapchainImage != VK_NULL_HANDLE) {
            TransitionImage(buffer, list.swapchainImage,
                            VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                            VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
                            VK_IMAGE_ASPECT_COLOR_BIT);
        }
        list.pass = CurrentPass::None;
        list.swapchainImage = VK_NULL_HANDLE;
    } else if (list.pass == CurrentPass::Compute) {
        list.pass = CurrentPass::None;
    }
}

void VulkanRHI::SetPipeline(rhi::CommandListHandle cmd, rhi::GraphicsPipelineHandle handle)
{
    VkCommandBuffer buffer = GetVkCommandBuffer(cmd);
    if (buffer == VK_NULL_HANDLE) {
        return;
    }
    if (handle.id >= m_Pipelines.size() || m_Pipelines[handle.id] == VK_NULL_HANDLE) {
        printf("SetPipeline: invalid graphics pipeline handle\n");
        return;
    }
    VulkanCommandList& list = m_CommandLists[cmd.id];
    if (handle.desc.layout.id < m_PipelineLayouts.size()) {
        list.boundLayout = m_PipelineLayouts[handle.desc.layout.id];
    }
    list.bindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    vkCmdBindPipeline(buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipelines[handle.id]);
}

void VulkanRHI::SetPipeline(rhi::CommandListHandle cmd, rhi::ComputePipelineHandle handle)
{
    VkCommandBuffer buffer = GetVkCommandBuffer(cmd);
    if (buffer == VK_NULL_HANDLE) {
        return;
    }
    if (handle.id >= m_ComputePipelines.size() ||
        m_ComputePipelines[handle.id] == VK_NULL_HANDLE) {
        printf("SetPipeline: invalid compute pipeline handle\n");
        return;
    }
    VulkanCommandList& list = m_CommandLists[cmd.id];
    if (handle.desc.layout.id < m_PipelineLayouts.size()) {
        list.boundLayout = m_PipelineLayouts[handle.desc.layout.id];
    }
    list.bindPoint = VK_PIPELINE_BIND_POINT_COMPUTE;
    vkCmdBindPipeline(buffer, VK_PIPELINE_BIND_POINT_COMPUTE, m_ComputePipelines[handle.id]);
}

void VulkanRHI::SetBindGroup(rhi::CommandListHandle cmd, std::uint32_t slot, rhi::BindGroupHandle handle)
{
    VkCommandBuffer buffer = GetVkCommandBuffer(cmd);
    if (buffer == VK_NULL_HANDLE) {
        return;
    }
    if (handle.id >= m_BindGroups.size() || m_BindGroups[handle.id] == VK_NULL_HANDLE) {
        printf("SetBindGroup: invalid bind group handle\n");
        return;
    }
    VulkanCommandList& list = m_CommandLists[cmd.id];
    if (list.boundLayout == VK_NULL_HANDLE) {
        printf("SetBindGroup: no pipeline bound yet\n");
        return;
    }
    vkCmdBindDescriptorSets(buffer, list.bindPoint, list.boundLayout,
                            slot, 1, &m_BindGroups[handle.id], 0, nullptr);
}

void VulkanRHI::SetVertexBuffer(rhi::CommandListHandle cmd, std::uint32_t slot, rhi::BufferHandle handle, std::size_t offset)
{
    VkCommandBuffer buffer = GetVkCommandBuffer(cmd);
    if (buffer == VK_NULL_HANDLE) {
        return;
    }
    if (handle.id >= m_Buffers.size() || m_Buffers[handle.id] == VK_NULL_HANDLE) {
        printf("SetVertexBuffer: invalid buffer handle\n");
        return;
    }
    if (offset > handle.desc.size) {
        printf("SetVertexBuffer offset is out of range\n");
        return;
    }
    VkDeviceSize vkOffset = static_cast<VkDeviceSize>(offset);
    vkCmdBindVertexBuffers(buffer, slot, 1, &m_Buffers[handle.id], &vkOffset);
}

void VulkanRHI::SetIndexBuffer(rhi::CommandListHandle cmd, rhi::BufferHandle handle, rhi::IndexFormat format, std::size_t offset)
{
    VkCommandBuffer buffer = GetVkCommandBuffer(cmd);
    if (buffer == VK_NULL_HANDLE) {
        return;
    }
    if (handle.id >= m_Buffers.size() || m_Buffers[handle.id] == VK_NULL_HANDLE) {
        printf("SetIndexBuffer: invalid buffer handle\n");
        return;
    }
    if (offset > handle.desc.size) {
        printf("SetIndexBuffer offset is out of range\n");
        return;
    }
    VkIndexType type = (format == rhi::IndexFormat::Uint16) ? VK_INDEX_TYPE_UINT16
                                                            : VK_INDEX_TYPE_UINT32;
    vkCmdBindIndexBuffer(buffer, m_Buffers[handle.id],
                         static_cast<VkDeviceSize>(offset), type);
}

void VulkanRHI::SetViewport(rhi::CommandListHandle cmd, float x, float y, float width, float height)
{
    VkCommandBuffer buffer = GetVkCommandBuffer(cmd);
    if (buffer == VK_NULL_HANDLE) {
        return;
    }
    VkViewport viewport{};
    viewport.x = x;
    viewport.y = y + height;
    viewport.width = width;
    viewport.height = -height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(buffer, 0, 1, &viewport);
}

void VulkanRHI::SetScissor(rhi::CommandListHandle cmd, std::uint32_t x, std::uint32_t y, std::uint32_t width, std::uint32_t height)
{
    VkCommandBuffer buffer = GetVkCommandBuffer(cmd);
    if (buffer == VK_NULL_HANDLE) {
        return;
    }
    VkRect2D scissor{};
    scissor.offset = { static_cast<std::int32_t>(x), static_cast<std::int32_t>(y) };
    scissor.extent = { width, height };
    vkCmdSetScissor(buffer, 0, 1, &scissor);
}

void VulkanRHI::Draw(rhi::CommandListHandle cmd, std::uint32_t vertexCount, std::uint32_t instanceCount,
                      std::uint32_t firstVertex, std::uint32_t firstInstance)
{
    VkCommandBuffer buffer = GetVkCommandBuffer(cmd);
    if (buffer == VK_NULL_HANDLE) {
        return;
    }
    vkCmdDraw(buffer, vertexCount, instanceCount, firstVertex, firstInstance);
}

void VulkanRHI::DrawIndexed(rhi::CommandListHandle cmd, std::uint32_t indexCount, std::uint32_t instanceCount,
                             std::uint32_t firstIndex, std::int32_t baseVertex,
                             std::uint32_t firstInstance)
{
    VkCommandBuffer buffer = GetVkCommandBuffer(cmd);
    if (buffer == VK_NULL_HANDLE) {
        return;
    }
    vkCmdDrawIndexed(buffer, indexCount, instanceCount, firstIndex, baseVertex, firstInstance);
}

void VulkanRHI::DrawIndirect(rhi::CommandListHandle cmd, rhi::BufferHandle params, std::size_t offset,
                              std::uint32_t drawCount, std::uint32_t stride)
{
    VkCommandBuffer buffer = GetVkCommandBuffer(cmd);
    if (buffer == VK_NULL_HANDLE) {
        return;
    }
    if (params.id >= m_Buffers.size() || m_Buffers[params.id] == VK_NULL_HANDLE) {
        printf("DrawIndirect: invalid params buffer handle\n");
        return;
    }
    vkCmdDrawIndirect(buffer, m_Buffers[params.id],
                      static_cast<VkDeviceSize>(offset), drawCount, stride);
}

void VulkanRHI::DrawIndexedIndirect(rhi::CommandListHandle cmd, rhi::BufferHandle params, std::size_t offset,
                                     std::uint32_t drawCount, std::uint32_t stride)
{
    VkCommandBuffer buffer = GetVkCommandBuffer(cmd);
    if (buffer == VK_NULL_HANDLE) {
        return;
    }
    if (params.id >= m_Buffers.size() || m_Buffers[params.id] == VK_NULL_HANDLE) {
        printf("DrawIndexedIndirect: invalid params buffer handle\n");
        return;
    }
    vkCmdDrawIndexedIndirect(buffer, m_Buffers[params.id],
                             static_cast<VkDeviceSize>(offset), drawCount, stride);
}

void VulkanRHI::Dispatch(rhi::CommandListHandle cmd, std::uint32_t x, std::uint32_t y, std::uint32_t z)
{
    VkCommandBuffer buffer = GetVkCommandBuffer(cmd);
    if (buffer == VK_NULL_HANDLE) {
        return;
    }
    vkCmdDispatch(buffer, x, y, z);
}

void VulkanRHI::DispatchIndirect(rhi::CommandListHandle cmd, rhi::BufferHandle params, std::size_t offset)
{
    VkCommandBuffer buffer = GetVkCommandBuffer(cmd);
    if (buffer == VK_NULL_HANDLE) {
        return;
    }
    if (params.id >= m_Buffers.size() || m_Buffers[params.id] == VK_NULL_HANDLE) {
        printf("DispatchIndirect: invalid params buffer handle\n");
        return;
    }
    vkCmdDispatchIndirect(buffer, m_Buffers[params.id], static_cast<VkDeviceSize>(offset));
}

void VulkanRHI::CopyBufferToBuffer(rhi::CommandListHandle cmd, rhi::BufferHandle src, rhi::BufferHandle dst, std::size_t size,
                                    std::size_t srcOffset, std::size_t dstOffset)
{
    VkCommandBuffer buffer = GetVkCommandBuffer(cmd);
    if (buffer == VK_NULL_HANDLE) {
        return;
    }
    if (src.id >= m_Buffers.size() || m_Buffers[src.id] == VK_NULL_HANDLE ||
        dst.id >= m_Buffers.size() || m_Buffers[dst.id] == VK_NULL_HANDLE) {
        printf("CopyBufferToBuffer: invalid buffer handle\n");
        return;
    }
    if (size == 0 || srcOffset > src.desc.size || size > src.desc.size - srcOffset ||
        dstOffset > dst.desc.size || size > dst.desc.size - dstOffset) {
        printf("CopyBufferToBuffer range is out of bounds\n");
        return;
    }
    VkBufferCopy region{};
    region.srcOffset = static_cast<VkDeviceSize>(srcOffset);
    region.dstOffset = static_cast<VkDeviceSize>(dstOffset);
    region.size = static_cast<VkDeviceSize>(size);
    vkCmdCopyBuffer(buffer, m_Buffers[src.id], m_Buffers[dst.id], 1, &region);
}

void VulkanRHI::CopyBufferToTexture(rhi::CommandListHandle cmd, rhi::BufferHandle src, rhi::TextureHandle dst,
                                     const rhi::TextureWriteDesc& region)
{
    VkCommandBuffer buffer = GetVkCommandBuffer(cmd);
    if (buffer == VK_NULL_HANDLE) {
        return;
    }
    if (src.id >= m_Buffers.size() || m_Buffers[src.id] == VK_NULL_HANDLE ||
        dst.id >= m_Textures.size() || m_Textures[dst.id] == VK_NULL_HANDLE) {
        printf("CopyBufferToTexture: invalid copy handle\n");
        return;
    }
    VkBufferImageCopy copy{};
    copy.bufferOffset = 0;
    copy.bufferRowLength = region.bytesPerRow == 0 ? 0 : region.bytesPerRow;
    copy.bufferImageHeight = region.rowsPerImage;
    copy.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    copy.imageSubresource.mipLevel = region.mipLevel;
    copy.imageSubresource.baseArrayLayer = 0;
    copy.imageSubresource.layerCount = 1;
    copy.imageOffset = { static_cast<std::int32_t>(region.x),
                         static_cast<std::int32_t>(region.y),
                         static_cast<std::int32_t>(region.z) };
    copy.imageExtent = { region.width, region.height, region.depth };
    vkCmdCopyBufferToImage(buffer, m_Buffers[src.id], m_Textures[dst.id],
                           VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy);
}

void VulkanRHI::CopyTextureToBuffer(rhi::CommandListHandle cmd, rhi::TextureHandle src, rhi::BufferHandle dst,
                                     const rhi::TextureWriteDesc& region)
{
    VkCommandBuffer buffer = GetVkCommandBuffer(cmd);
    if (buffer == VK_NULL_HANDLE) {
        return;
    }
    if (src.id >= m_Textures.size() || m_Textures[src.id] == VK_NULL_HANDLE ||
        dst.id >= m_Buffers.size() || m_Buffers[dst.id] == VK_NULL_HANDLE) {
        printf("CopyTextureToBuffer: invalid copy handle\n");
        return;
    }
    VkBufferImageCopy copy{};
    copy.bufferOffset = 0;
    copy.bufferRowLength = region.bytesPerRow == 0 ? 0 : region.bytesPerRow;
    copy.bufferImageHeight = region.rowsPerImage;
    copy.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    copy.imageSubresource.mipLevel = region.mipLevel;
    copy.imageSubresource.baseArrayLayer = 0;
    copy.imageSubresource.layerCount = 1;
    copy.imageOffset = { static_cast<std::int32_t>(region.x),
                         static_cast<std::int32_t>(region.y),
                         static_cast<std::int32_t>(region.z) };
    copy.imageExtent = { region.width, region.height, region.depth };
    vkCmdCopyImageToBuffer(buffer, m_Textures[src.id],
                           VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                           m_Buffers[dst.id], 1, &copy);
}

void VulkanRHI::Submit(std::span<const rhi::CommandListHandle> cmds)
{
    if (m_Device == VK_NULL_HANDLE || cmds.empty() || m_CurrentFrame == 0xFFFFFFFF) {
        return;
    }
    FrameContext& frame = m_Frames[m_CurrentFrame];

    std::vector<VkCommandBuffer> buffers;
    buffers.reserve(cmds.size());
    for (rhi::CommandListHandle handle : cmds) {
        if (handle.id >= m_CommandLists.size()) {
            return;
        }
        VulkanCommandList& list = m_CommandLists[handle.id];
        if (!list.open || list.frame != m_CurrentFrame ||
            list.bufferIndex >= frame.cmds.size()) {
            return;
        }
        VkCommandBuffer buffer = frame.cmds[list.bufferIndex];
        if (list.pass != CurrentPass::None) {
            printf("Submit: command list still has an open pass, call EndPass first\n");
            return;
        }
        if (vkEndCommandBuffer(buffer) != VK_SUCCESS) {
            return;
        }
        buffers.push_back(buffer);
    }

    VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = &frame.imageAvailable;
    submitInfo.pWaitDstStageMask = &waitStage;
    submitInfo.commandBufferCount = static_cast<std::uint32_t>(buffers.size());
    submitInfo.pCommandBuffers = buffers.data();
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &frame.renderFinished;

    if (vkQueueSubmit(m_GraphicsQueue, 1, &submitInfo, frame.inFlight) != VK_SUCCESS) {
        return;
    }

    for (rhi::CommandListHandle handle : cmds) {
        m_CommandLists[handle.id].open = false;
        m_FreeCommandLists.push_back(handle.id);
    }
}
