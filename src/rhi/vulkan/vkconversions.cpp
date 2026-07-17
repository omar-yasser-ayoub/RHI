#include "rhi/vulkan/vkrhi.h"

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
