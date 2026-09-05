#include "rhi/webgpu/wgpurhi.h"

rhi::BindGroupLayoutHandle WebGPURHI::CreateBindGroupLayout(const rhi::BindGroupLayoutDesc& desc)
{
    // we have many shaderBindings, each with a list of bindings,
    // so we need to make them first into one structure then iterate over it
    // to create the bind group layout, then return the handle

    rhi::BindGroupLayoutHandle handle;
    handle.id = 0xFFFFFFFF;

    if (desc.shaderBindings.empty())
        return handle;

    // for a bind group layout, all bindings must belong to the same set.
    // while iterating, we need to keep track of the set and reject the input
    // if any bindings belong to a different set
    wgpu::BindGroupLayoutDescriptor layoutDesc{};

    std::vector<wgpu::BindGroupLayoutEntry> entries;
    entries.reserve(desc.shaderBindings.size());

    int currentSet = -1;

    for (const auto& shaderBinding : desc.shaderBindings)
    {
        if (currentSet == -1)
            currentSet = shaderBinding.set;
        else if (currentSet != shaderBinding.set)
            return handle;

        wgpu::BindGroupLayoutEntry entry{};

        wgpu::ShaderStage visibility = wgpu::ShaderStage::None;

        if (HasFlag(shaderBinding.stage, rhi::ShaderStageFlags::Vertex))
            visibility |= wgpu::ShaderStage::Vertex;

        if (HasFlag(shaderBinding.stage, rhi::ShaderStageFlags::Fragment))
            visibility |= wgpu::ShaderStage::Fragment;

        if (HasFlag(shaderBinding.stage, rhi::ShaderStageFlags::Compute))
            visibility |= wgpu::ShaderStage::Compute;

        entry.visibility = visibility;

        switch (shaderBinding.type)
        {
        case rhi::BindingType::UniformBuffer:
            entry.buffer.type = wgpu::BufferBindingType::Uniform;
            break;

        case rhi::BindingType::StorageBuffer:
            entry.buffer.type = wgpu::BufferBindingType::Storage;
            break;

        case rhi::BindingType::SampledTexture:
            entry.texture.sampleType = wgpu::TextureSampleType::Float;
            entry.texture.viewDimension = wgpu::TextureViewDimension::e2D;
            break;

        case rhi::BindingType::StorageTexture:
            entry.storageTexture.access =
                wgpu::StorageTextureAccess::WriteOnly;
            entry.storageTexture.format =
                wgpu::TextureFormat::RGBA8Unorm;
            entry.storageTexture.viewDimension =
                wgpu::TextureViewDimension::e2D;
            break;

        case rhi::BindingType::Sampler:
            entry.sampler.type = wgpu::SamplerBindingType::Filtering;
            break;

        default:
            return handle;
        }

        entries.push_back(entry);
    }

    layoutDesc.entryCount = entries.size();
    layoutDesc.entries = entries.data();

    wgpu::BindGroupLayout bindGroupLayout =
        m_Device.CreateBindGroupLayout(&layoutDesc);

    if (!bindGroupLayout)
        return handle;

    handle.id = m_BindGroupLayouts.size();
    m_BindGroupLayouts.push_back(bindGroupLayout);

    return handle;
}

rhi::BindGroupHandle WebGPURHI::CreateBindGroup(const rhi::BindGroupDesc& desc)
{
    rhi::BindGroupHandle handle;
    handle.id = 0xFFFFFFFF;

    if (desc.layout.id >= m_BindGroupLayouts.size())
        return handle;
    if (desc.entries.empty())
        return handle;
    std::vector<wgpu::BindGroupEntry> entries;
    entries.reserve(desc.entries.size());
    for (const auto& binding : desc.entries)
    {
        wgpu::BindGroupEntry entry;
        entry.binding = binding.binding;

        if (binding.buffer.id != 0xFFFFFFFF)
        {
            if (binding.buffer.id >= m_Buffers.size())
                return handle;
            entry.buffer = m_Buffers[binding.buffer.id];
        }
        else if (binding.textureView.id != 0xFFFFFFFF)
        {
            if (binding.textureView.id >= m_TextureViews.size())
                return handle;
            entry.textureView = m_TextureViews[binding.textureView.id];
        }
        else if (binding.sampler.id != 0xFFFFFFFF)
        {
            if (binding.sampler.id >= m_Samplers.size())
                return handle;
            entry.sampler = m_Samplers[binding.sampler.id];
        }

        entries.push_back(entry);
    }

    wgpu::BindGroupDescriptor bindGroupDesc{};
    bindGroupDesc.layout = m_BindGroupLayouts[desc.layout.id];
    bindGroupDesc.entryCount = entries.size();
    bindGroupDesc.entries = entries.data();

    wgpu::BindGroup bindGroup = m_Device.CreateBindGroup(&bindGroupDesc);

    if (!bindGroup)
        return handle;

    handle.id = m_BindGroups.size();
    m_BindGroups.push_back(bindGroup);
    return handle;
}
