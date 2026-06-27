#include <cstdint>

enum class Filter
{
    Nearest,
    Linear,
};

enum class MipmapFilter
{
    Nearest,
    Linear,
};

enum class AddressMode
{
    Repeat,
    MirrorRepeat,
    ClampToEdge,
    ClampToBorder,
};

struct SamplerDesc
{
    Filter minFilter = Filter::Linear;
    Filter magFilter = Filter::Linear;
    MipmapFilter mipFilter = MipmapFilter::Linear;

    AddressMode addressU = AddressMode::Repeat;
    AddressMode addressV = AddressMode::Repeat;
    AddressMode addressW = AddressMode::Repeat;

    float minLOD = 0.0f;
    float maxLOD = 32.0f;

    bool anisotropyEnabled = false;
    uint32_t maxAnisotropy = 1;
};
