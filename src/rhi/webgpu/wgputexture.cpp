#include "rhi/webgpu/wgpurhi.h"

rhi::TextureHandle WebGPURHI::CreateTexture(const rhi::TextureDesc& desc) {
    return rhi::TextureHandle{};
}

void WebGPURHI::DestroyTexture(rhi::TextureHandle handle) {
    return;
}

void WebGPURHI::WriteTexture(rhi::TextureHandle handle, const rhi::TextureWriteDesc& desc, const void* data) {
    return;
}
