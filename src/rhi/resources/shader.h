#include <cstdint>
#include <vector>

namespace rhi {

    enum class BindingType
    {
        UniformBuffer,
        StorageBuffer,
        SampledTexture,
        StorageTexture,
        Sampler,
    };

    enum class ShaderStageFlags
    {
        Vertex = 1 << 0,
        Fragment = 1 << 1,
        Compute = 1 << 2,
    };

    struct ShaderBinding
    {
        uint32_t set;
        uint32_t binding;
        BindingType type;
        uint32_t arraySize;
        ShaderStageFlags stage;
    };

    enum class VertexFormat
    {
        Float32,
        Float32x2,
        Float32x3,
        Float32x4,
        Int32,
        UInt32,
        Int8,
        UInt8,
    };

    struct VertexInputAttr
    {
        uint32_t location;
        VertexFormat format;
        uint32_t stride;
        uint32_t offset;
    };

    struct ShaderReflection
    {
        std::vector<ShaderBinding> bindings;
        std::vector<VertexInputAttr> vertexInputs;
    };

    struct ShaderModule
    {
        std::vector<uint8_t> bytecode;
        ShaderReflection reflection;
    };
}
