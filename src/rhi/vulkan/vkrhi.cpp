#include "rhi/vulkan/vkrhi.h"
#include "rhi/irhi.h"
#include "rhi/resources/buffer.h"
#include <vulkan/vulkan.h>
#include <iostream>
#include <vector>


bool VulkanRHI::Initialize() {
    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "reeka's RHI";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "N/A";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_3;

    VkInstanceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    VkResult result = vkCreateInstance(&createInfo, nullptr, &m_Instance);
    if (result != VK_SUCCESS) {
        std::cerr << "Failed to create Vulkan instance: " << result << std::endl;
        return false;
    }

    uint32_t deviceCount = 0;

    result = vkEnumeratePhysicalDevices(m_Instance, &deviceCount, nullptr);
    if (result != VK_SUCCESS) {
        std::cerr << "Failed to enumerate Vulkan devices: " << result << std::endl;
        return false;
    }
    if (deviceCount == 0) {
        std::cerr << "No Vulkan devices found." << std::endl;
        return false;
    }

    std::vector<VkPhysicalDevice> devices(deviceCount);
    result = vkEnumeratePhysicalDevices(m_Instance, &deviceCount, devices.data());

    if (result != VK_SUCCESS) {
        std::cerr << "Failed to enumerate Vulkan devices: " << result << std::endl;
        vkDestroyInstance(m_Instance, nullptr);
        m_Instance = VK_NULL_HANDLE;
        return false;
    }

    m_PhysicalDevice = devices[0];

    VkPhysicalDeviceProperties deviceProperties;
    vkGetPhysicalDeviceProperties(m_PhysicalDevice, &deviceProperties);

    m_Capabilities.deviceName = deviceProperties.deviceName;

    VkPhysicalDeviceMeshShaderFeaturesEXT meshFeatures{
        VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_FEATURES_EXT
    };

    VkPhysicalDeviceRayTracingPipelineFeaturesKHR rtFeatures{
        VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR
    };

    VkPhysicalDeviceFeatures2 features2{
        VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2
    };

    features2.pNext = &meshFeatures;
    meshFeatures.pNext = &rtFeatures;

    vkGetPhysicalDeviceFeatures2(m_PhysicalDevice, &features2);

    m_Capabilities.SupportsMeshShaders = meshFeatures.meshShader;
    m_Capabilities.SupportsRayTracing = rtFeatures.rayTracingPipeline;

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(
        m_PhysicalDevice,
        &queueFamilyCount,
        nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);

    vkGetPhysicalDeviceQueueFamilyProperties(
        m_PhysicalDevice,
        &queueFamilyCount,
        queueFamilies.data());

    uint32_t graphicsQueueFamily = UINT32_MAX;

    for (uint32_t i = 0; i < queueFamilyCount; ++i)
    {
        if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            graphicsQueueFamily = i;
            break;
        }
    }

    if (graphicsQueueFamily == UINT32_MAX) {
        std::cerr << "No graphics queue family found.\n";
        vkDestroyInstance(m_Instance, nullptr);
        m_Instance = VK_NULL_HANDLE;
        m_PhysicalDevice = VK_NULL_HANDLE;
        return false;
    }

    float priority = 1.0f;

    VkDeviceQueueCreateInfo queueInfo{};
    queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueInfo.queueFamilyIndex = graphicsQueueFamily;
    queueInfo.queueCount = 1;
    queueInfo.pQueuePriorities = &priority;

    VkDeviceCreateInfo deviceInfo{};
    deviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceInfo.queueCreateInfoCount = 1;
    deviceInfo.pQueueCreateInfos = &queueInfo;

    result = vkCreateDevice(
        m_PhysicalDevice,
        &deviceInfo,
        nullptr,
        &m_Device);

    if (result != VK_SUCCESS) {
        std::cerr << "Failed to create Vulkan device: " << result << '\n';
        vkDestroyInstance(m_Instance, nullptr);
        m_Instance = VK_NULL_HANDLE;
        m_PhysicalDevice = VK_NULL_HANDLE;
        return false;
    }

    return true;
}

void VulkanRHI::Shutdown() {
    if (m_Device) {
        vkDeviceWaitIdle(m_Device);

        for (std::size_t i = 0; i < m_Buffers.size(); ++i) {
            if (m_Buffers[i] != VK_NULL_HANDLE) {
                vkDestroyBuffer(m_Device, m_Buffers[i], nullptr);
                m_Buffers[i] = VK_NULL_HANDLE;
            }
            if (i < m_BufferMemory.size() && m_BufferMemory[i] != VK_NULL_HANDLE) {
                vkFreeMemory(m_Device, m_BufferMemory[i], nullptr);
                m_BufferMemory[i] = VK_NULL_HANDLE;
            }
        }
        for (VkSampler sampler : m_Samplers) {
            if (sampler != VK_NULL_HANDLE) {
                vkDestroySampler(m_Device, sampler, nullptr);
            }
        }

        m_Buffers.clear();
        m_BufferMemory.clear();
        m_Samplers.clear();
        vkDestroyDevice(m_Device, nullptr);
        m_Device = VK_NULL_HANDLE;
    }

    if (m_Instance) {
        vkDestroyInstance(m_Instance, nullptr);
        m_Instance = VK_NULL_HANDLE;
    }
    m_PhysicalDevice = VK_NULL_HANDLE;
}

VkFilter VulkanRHI::toVk(rhi::Filter mode){
    switch (mode) {
        case rhi::Filter::Nearest: return VK_FILTER_NEAREST;
        case rhi::Filter::Linear: return VK_FILTER_LINEAR;
    }
}

VkSamplerMipmapMode VulkanRHI::toVk(rhi::MipmapFilter mode){
    switch (mode) {
        case rhi::MipmapFilter::Nearest: return VK_SAMPLER_MIPMAP_MODE_NEAREST;
        case rhi::MipmapFilter::Linear: return VK_SAMPLER_MIPMAP_MODE_LINEAR;
    }
}

VkSamplerAddressMode VulkanRHI::toVk(rhi::AddressMode mode){
    switch (mode) {
        case rhi::AddressMode::Repeat: return VK_SAMPLER_ADDRESS_MODE_REPEAT;
        case rhi::AddressMode::MirrorRepeat: return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
        case rhi::AddressMode::ClampToEdge: return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        case rhi::AddressMode::ClampToBorder: return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
        case rhi::AddressMode::MirrorClampToEdge: return VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE;
    }
}

VkCompareOp VulkanRHI::toVk(rhi::CompareOp mode){
    switch (mode) {
        case rhi::CompareOp::Never: return VK_COMPARE_OP_NEVER;
        case rhi::CompareOp::Less: return VK_COMPARE_OP_LESS;
        case rhi::CompareOp::Equal: return VK_COMPARE_OP_EQUAL;
        case rhi::CompareOp::LessEqual: return VK_COMPARE_OP_LESS_OR_EQUAL;
        case rhi::CompareOp::Greater: return VK_COMPARE_OP_GREATER;
        case rhi::CompareOp::NotEqual: return VK_COMPARE_OP_NOT_EQUAL;
        case rhi::CompareOp::GreaterEqual: return VK_COMPARE_OP_GREATER_OR_EQUAL;
        case rhi::CompareOp::Always: return VK_COMPARE_OP_ALWAYS;
    }
}

VkBorderColor VulkanRHI::toVk(rhi::BorderColor mode){
    switch (mode) {
        case rhi::BorderColor::TransparentBlack: return VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
        case rhi::BorderColor::OpaqueBlack: return VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
        case rhi::BorderColor::OpaqueWhite: return VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
    }
}

VkBufferUsageFlags VulkanRHI::toVk(rhi::BufferUsage usage) {
    VkBufferUsageFlags flags = 0;
    if (rhi::HasFlag(usage, rhi::BufferUsage::Vertex)) flags |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    if (rhi::HasFlag(usage, rhi::BufferUsage::Index)) flags |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    if (rhi::HasFlag(usage, rhi::BufferUsage::Uniform)) flags |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    if (rhi::HasFlag(usage, rhi::BufferUsage::Storage)) flags |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
    if (rhi::HasFlag(usage, rhi::BufferUsage::Indirect)) flags |= VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;
    if (rhi::HasFlag(usage, rhi::BufferUsage::CopySrc)) flags |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    if (rhi::HasFlag(usage, rhi::BufferUsage::CopyDst)) flags |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    return flags;
}

VkMemoryPropertyFlags VulkanRHI::toVk(rhi::MemoryUsage usage)
{
    switch (usage)
    {
        case rhi::MemoryUsage::GpuOnly:
            return VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
        case rhi::MemoryUsage::CpuToGpu:
            return VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
                 | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
        case rhi::MemoryUsage::GpuToCpu:
            return VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
                 | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
        default:
            return 0;
    }
}

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

rhi::SamplerHandle VulkanRHI::CreateSampler(const rhi::SamplerDesc& desc) {
    VkSamplerCreateInfo samplerDesc = {};
    samplerDesc.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerDesc.magFilter = toVk(desc.magFilter);
    samplerDesc.minFilter = toVk(desc.minFilter);
    samplerDesc.mipmapMode = toVk(desc.mipFilter);
    samplerDesc.addressModeU = toVk(desc.addressU);
    samplerDesc.addressModeV = toVk(desc.addressV);
    samplerDesc.addressModeW = toVk(desc.addressW);
    samplerDesc.mipLodBias = desc.mipLodBias;
    samplerDesc.anisotropyEnable = desc.anisotropyEnabled;
    samplerDesc.maxAnisotropy = desc.maxAnisotropy;
    samplerDesc.compareEnable = desc.compareEnabled;
    samplerDesc.compareOp = toVk(desc.compareOp);
    samplerDesc.minLod = desc.minLOD;
    samplerDesc.maxLod = desc.maxLOD;
    samplerDesc.borderColor = toVk(desc.borderColor);

    VkSampler sampler;

    if (vkCreateSampler(m_Device, &samplerDesc, nullptr, &sampler) != VK_SUCCESS) {
        printf("[ERROR] Failed to create sampler\n");
        return rhi::SamplerHandle{};
    }

    rhi::SamplerHandle handle;
    handle.id = static_cast<uint64_t>(m_Samplers.size());
    m_Samplers.push_back(sampler);
    return handle;
}

rhi::TextureHandle VulkanRHI::CreateTexture(const rhi::TextureDesc& desc) {
    return rhi::TextureHandle{};
}

rhi::BufferHandle VulkanRHI::CreateBuffer(const rhi::BufferDesc& desc) {
    VkBufferCreateInfo bufferDesc{};

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

    m_BufferMemory.push_back(bufferMemory);
    m_Buffers.push_back(buffer);
    rhi::BufferHandle handle;
    handle.id = static_cast<uint64_t>(m_Buffers.size() - 1);

    return handle;
}

void VulkanRHI::DestroySampler(rhi::SamplerHandle handle) {
    if (!m_Device || handle.id == 0xFFFFFFFFFFFFFFFF ||
        handle.id >= m_Samplers.size() ||
        m_Samplers[handle.id] == VK_NULL_HANDLE) {
        return;
    }
    vkDestroySampler(m_Device, m_Samplers[handle.id], nullptr);
    m_Samplers[handle.id] = VK_NULL_HANDLE;
}

void VulkanRHI::DestroyTexture(rhi::TextureHandle handle) {
    return;
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
    vkUnmapMemory(m_Device, m_BufferMemory[handle.id]);
}

void VulkanRHI::WriteBuffer(rhi::BufferHandle handle, const void* data, std::size_t size, std::size_t offset) {
    return;
}

void VulkanRHI::WriteTexture(rhi::TextureHandle handle, const rhi::TextureWriteDesc& desc, const void* data) {
    return;
}
