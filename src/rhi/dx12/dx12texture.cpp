#include "rhi/dx12/dx12rhi.h"


rhi::TextureHandle DX12RHI::CreateTexture(const rhi::TextureDesc& desc) {
    return rhi::TextureHandle{};
}

void DX12RHI::DestroyTexture(rhi::TextureHandle handle) {
    return;
}

void DX12RHI::WriteTexture(rhi::TextureHandle handle, const rhi::TextureWriteDesc& desc, const void* data) {
    return;
}
