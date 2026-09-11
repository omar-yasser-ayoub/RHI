#include "rhi/vulkan/vkrhi.h"

uint32_t VulkanRHI::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
    VkPhysicalDeviceMemoryProperties memProps;
    vkGetPhysicalDeviceMemoryProperties(m_PhysicalDevice, &memProps);

    for (uint32_t i = 0; i < memProps.memoryTypeCount; i++)
    {
        if ((typeFilter & (1 << i)) &&
            (memProps.memoryTypes[i].propertyFlags & properties) == properties)
            return i;
    }

    printf("[ERROR] Failed to find suitable memory type\n");
    return UINT32_MAX;
}

rhi::BufferHandle VulkanRHI::CreateBuffer(const rhi::BufferDesc& desc) {
    VkBufferCreateInfo bufferDesc{};

    if (desc.size == 0) {
        printf("[ERROR] Buffer size must be greater than 0\n");
        return rhi::BufferHandle{};
    }

    bufferDesc.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferDesc.size = desc.size;
    bufferDesc.usage = toVk(desc.usage);
    bufferDesc.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VkBuffer buffer;
    if (vkCreateBuffer(m_Device, &bufferDesc, nullptr, &buffer) != VK_SUCCESS) {
        printf("[ERROR] Failed to create buffer\n");
        return rhi::BufferHandle{};
    }

    VkMemoryRequirements memReqs;
    vkGetBufferMemoryRequirements(m_Device, buffer, &memReqs);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memReqs.size;
    uint32_t memTypeIndex = findMemoryType(memReqs.memoryTypeBits, toVk(desc.memoryUsage));
    if (memTypeIndex == UINT32_MAX)
    {
        printf("[ERROR] Failed to find suitable memory type\n");
        vkDestroyBuffer(m_Device, buffer, nullptr);
        return rhi::BufferHandle{};
    }
    allocInfo.memoryTypeIndex = memTypeIndex;

    VkDeviceMemory bufferMemory;
    if (vkAllocateMemory(m_Device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
        printf("[ERROR] Failed to allocate buffer memory\n");
        vkDestroyBuffer(m_Device, buffer, nullptr);
        return rhi::BufferHandle{};
    }

    if (vkBindBufferMemory(m_Device, buffer, bufferMemory, 0) != VK_SUCCESS) {
        printf("[ERROR] Failed to bind buffer memory\n");
        vkFreeMemory(m_Device, bufferMemory, nullptr);
        vkDestroyBuffer(m_Device, buffer, nullptr);
        return rhi::BufferHandle{};
    }

    rhi::BufferHandle handle;
    handle.desc = desc;
    if (!m_FreeBuffers.empty()) {
        handle.id = m_FreeBuffers.back();
        m_FreeBuffers.pop_back();
        m_BufferMemory[handle.id] = bufferMemory;
        m_Buffers[handle.id] = buffer;
    } else {
        handle.id = static_cast<uint64_t>(m_Buffers.size());
        m_BufferMemory.push_back(bufferMemory);
        m_Buffers.push_back(buffer);
    }

    return handle;
}

void VulkanRHI::DestroyBuffer(rhi::BufferHandle handle) {
    if (!m_Device || handle.id == 0xFFFFFFFFFFFFFFFF ||
        handle.id >= m_Buffers.size() ||
        handle.id >= m_BufferMemory.size() ||
        m_Buffers[handle.id] == VK_NULL_HANDLE) {
        return;
    }

    vkDestroyBuffer(m_Device, m_Buffers[handle.id], nullptr);
    m_Buffers[handle.id] = VK_NULL_HANDLE;
    if (m_BufferMemory[handle.id] != VK_NULL_HANDLE) {
        vkFreeMemory(m_Device, m_BufferMemory[handle.id], nullptr);
        m_BufferMemory[handle.id] = VK_NULL_HANDLE;
    }
    m_FreeBuffers.push_back(handle.id);
}

void* VulkanRHI::MapBuffer(rhi::BufferHandle handle) {
    void* ptr = nullptr;
    if (!m_Device || handle.id == 0xFFFFFFFFFFFFFFFF ||
        handle.id >= m_Buffers.size() ||
        handle.id >= m_BufferMemory.size() ||
        m_Buffers[handle.id] == VK_NULL_HANDLE ||
        m_BufferMemory[handle.id] == VK_NULL_HANDLE) {
        return ptr;
    }
    if (handle.desc.memoryUsage == rhi::MemoryUsage::GpuOnly) {
        printf("[ERROR] Cannot map a GPU-only buffer\n");
        return nullptr;
    }

    VkDeviceMemory memory = m_BufferMemory[handle.id];

    if (vkMapMemory(m_Device, memory, 0, VK_WHOLE_SIZE, 0, &ptr) != VK_SUCCESS)
    {
        printf("[ERROR] Failed to map buffer\n");
        return nullptr;
    }

    return ptr;
}

void VulkanRHI::UnmapBuffer(rhi::BufferHandle handle) {
    if (!m_Device || handle.id == 0xFFFFFFFFFFFFFFFF ||
        handle.id >= m_BufferMemory.size() ||
        m_BufferMemory[handle.id] == VK_NULL_HANDLE) {
        return;
    }
    if (handle.desc.memoryUsage == rhi::MemoryUsage::GpuOnly) {
        printf("[ERROR] Cannot unmap a GPU-only buffer\n");
        return;
    }
    vkUnmapMemory(m_Device, m_BufferMemory[handle.id]);
}

void VulkanRHI::WriteBuffer(rhi::BufferHandle handle, const void* data, std::size_t size, std::size_t offset) {
    if (!m_Device || !data || size == 0 ||
        handle.id == 0xFFFFFFFFFFFFFFFF ||
        handle.id >= m_Buffers.size() ||
        handle.id >= m_BufferMemory.size() ||
        m_Buffers[handle.id] == VK_NULL_HANDLE ||
        m_BufferMemory[handle.id] == VK_NULL_HANDLE) {
        return;
    }

    if (offset > handle.desc.size || size > handle.desc.size - offset) {
        printf("[ERROR] WriteBuffer range is outside the Vulkan buffer\n");
        return;
    }
    if (handle.desc.memoryUsage == rhi::MemoryUsage::GpuOnly) {
        printf("[ERROR] GPU-only Vulkan buffer writes require a staging buffer\n");
        return;
    }

    void* mapped = nullptr;
    if (vkMapMemory(m_Device, m_BufferMemory[handle.id], offset, size, 0, &mapped) != VK_SUCCESS) {
        printf("[ERROR] Failed to map Vulkan buffer for writing\n");
        return;
    }

    std::memcpy(mapped, data, size);
    vkUnmapMemory(m_Device, m_BufferMemory[handle.id]);
}
