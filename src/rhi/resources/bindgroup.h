#pragma once
#include "rhi/resources/buffer.h"
#include "rhi/resources/sampler.h"
#include "rhi/resources/shader.h"
#include "rhi/resources/texture.h"
#include <vector>
namespace rhi {
    struct BindGroupEntry
    {
        uint32_t binding;
        BufferHandle buffer;
        TextureViewHandle textureView;
        SamplerHandle sampler;
    };

    struct BindGroupLayoutDesc {
        std::vector<ShaderBinding> shaderBindings;
    };

    struct BindGroupLayoutHandle
    {
        std::uint64_t id = 0xFFFFFFFF;
        BindGroupLayoutDesc desc;
    };

    struct BindGroupDesc
    {
        BindGroupLayoutHandle layout;
        std::vector<BindGroupEntry> entries;
    };

    struct BindGroupHandle
    {
        std::uint64_t id = 0xFFFFFFFF;
        BindGroupDesc desc;
    };

}
