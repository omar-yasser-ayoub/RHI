#include "rhi/vulkan/vkrhi.h"

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
