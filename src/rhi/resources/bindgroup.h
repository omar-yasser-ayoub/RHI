#pragma once
#include "rhi/resources/buffer.h"
#include "rhi/resources/sampler.h"
#include "rhi/resources/shader.h"
#include "rhi/resources/texture.h"
#include <vector>
namespace rhi {
    struct BindGroupLayoutHandle
    {
        std::uint64_t id = 0xFFFFFFFF;
    };

    struct BindGroupHandle
    {
        std::uint64_t id = 0xFFFFFFFF;
    };

    struct BindGroupEntry
    {
        uint32_t binding;
        BufferHandle buffer;
        TextureViewHandle textureView;
        SamplerHandle sampler;
    };

    struct BindGroupDesc
    {
        BindGroupLayoutHandle layout;
        std::vector<BindGroupEntry> entries;
    };

    struct BindGroupLayoutDesc {
        std::vector<ShaderBinding> shaderBindings;
    };

}
