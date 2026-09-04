#include "rhi/resources/texture.h"
#include "rhi/webgpu/wgpurhi.h"

rhi::TextureHandle WebGPURHI::CreateTexture(const rhi::TextureDesc& desc) {
    wgpu::TextureDescriptor texDesc{};

    texDesc.size = wgpu::Extent3D{ desc.width, desc.height, desc.depth };
    texDesc.mipLevelCount = desc.mipLevels;
    texDesc.sampleCount = toWebGPU(desc.sampleCount);
    texDesc.dimension = toWebGPU(desc.dimension);
    texDesc.format = toWebGPU(desc.format);
    texDesc.usage = toWebGPU(desc.usage);

    // for now, I've chosen to ignore viewFormats and TextureViewDimension, both since they are not required,
    // but also since color space reinterpretation isnt something i particularly care about right now.

    wgpu::Texture texture = m_Device.CreateTexture(&texDesc);

    if (!texture) {
        return rhi::TextureHandle{};
    }

    m_Textures.push_back(texture);

    rhi::TextureHandle handle{};
    handle.id = static_cast<std::uint64_t>(m_Textures.size() - 1);

    return handle;
}

rhi::TextureViewHandle WebGPURHI::CreateTextureView(const rhi::TextureViewDesc& desc, rhi::TextureHandle texture) {
    wgpu::TextureViewDescriptor viewDesc{};
    viewDesc.baseMipLevel = desc.baseMipLevel;
    viewDesc.baseArrayLayer = desc.baseArrayLayer;
    viewDesc.mipLevelCount = desc.mipLevels;
    viewDesc.arrayLayerCount = desc.arrayLayers;
    viewDesc.format = toWebGPU(desc.format);

    wgpu::TextureView view = m_Textures[texture.id].CreateView(&viewDesc);

    if (!view) {
        return rhi::TextureViewHandle{};
    }

    m_TextureViews.push_back(view);

    rhi::TextureViewHandle handle{};
    handle.id = static_cast<std::uint64_t>(m_TextureViews.size() - 1);

    return handle;
}

void WebGPURHI::DestroyTexture(rhi::TextureHandle handle) {
    if (handle.id >= m_Textures.size() || !m_Textures[handle.id]) {
        return;
    }

    m_Textures[handle.id].Destroy();
    m_Textures[handle.id] = nullptr;
    return;
}

void WebGPURHI::DestroyTextureView(rhi::TextureViewHandle handle) {
    if (handle.id >= m_TextureViews.size() || !m_TextureViews[handle.id]) {
        return;
    }

    m_TextureViews[handle.id] = {};
    return;
}

void WebGPURHI::WriteTexture(rhi::TextureHandle handle, const rhi::TextureWriteDesc& desc, const void* data) {
    // TODO: Implement texture write
    return;
}
