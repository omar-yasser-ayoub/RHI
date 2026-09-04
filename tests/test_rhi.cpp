#include <gtest/gtest.h>
#include "rhi/IRHI.h"
#include <array>
#include <cstdint>
#include <cstring>
#include <vector>

#if defined(RHI_VULKAN) || defined(RHI_DX12)
    #include <Windows.h>
    #include <wrl/client.h>
    #include <third_party/dxc/inc/dxcapi.h>

#endif


#if defined(RHI_VULKAN)
    #include "rhi/vulkan/vkrhi.h"
    using RHIImpl = VulkanRHI;
#elif defined(RHI_DX12)
    #include "rhi/dx12/dx12rhi.h"
    using RHIImpl = DX12RHI;
#elif defined(RHI_WEBGPU)
    #include "rhi/webgpu/wgpurhi.h"
    using RHIImpl = WebGPURHI;
#else
    #error "No RHI backend selected"
#endif

class RHITest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        ASSERT_TRUE(rhi.Initialize()) << "Failed to initialize RHI";
    }

    void TearDown() override
    {
        rhi.Shutdown();
    }

    RHIImpl rhi;
};

// ============================================================================
// Initialization Tests
// ============================================================================

TEST_F(RHITest, InitializeSuccessfully)
{
    // Test is implicit: SetUp() will fail if Initialize() returns false
    EXPECT_TRUE(true);
}

// ============================================================================
// Sampler Creation Tests
// ============================================================================

TEST_F(RHITest, CreateDefaultSampler)
{
    rhi::SamplerDesc desc;
    auto handle = rhi.CreateSampler(desc);

    EXPECT_NE(handle.id, 0xFFFFFFFFFFFFFFFF) << "Sampler handle should be valid";
}

TEST_F(RHITest, CreateMultipleSamplers)
{
    rhi::SamplerDesc desc;

    auto handle1 = rhi.CreateSampler(desc);
    auto handle2 = rhi.CreateSampler(desc);
    auto handle3 = rhi.CreateSampler(desc);

    EXPECT_NE(handle1.id, 0xFFFFFFFFFFFFFFFF);
    EXPECT_NE(handle2.id, 0xFFFFFFFFFFFFFFFF);
    EXPECT_NE(handle3.id, 0xFFFFFFFFFFFFFFFF);

    // Handles should be different
    EXPECT_NE(handle1.id, handle2.id);
    EXPECT_NE(handle2.id, handle3.id);
}

TEST_F(RHITest, CreateSamplerWithNearestFiltering)
{
    rhi::SamplerDesc desc;
    desc.minFilter = rhi::Filter::Nearest;
    desc.magFilter = rhi::Filter::Nearest;
    desc.mipFilter = rhi::MipmapFilter::Nearest;

    auto handle = rhi.CreateSampler(desc);
    EXPECT_NE(handle.id, 0xFFFFFFFFFFFFFFFF);
}

TEST_F(RHITest, CreateSamplerWithLinearFiltering)
{
    rhi::SamplerDesc desc;
    desc.minFilter = rhi::Filter::Linear;
    desc.magFilter = rhi::Filter::Linear;
    desc.mipFilter = rhi::MipmapFilter::Linear;

    auto handle = rhi.CreateSampler(desc);
    EXPECT_NE(handle.id, 0xFFFFFFFFFFFFFFFF);
}

TEST_F(RHITest, CreateSamplerWithDifferentAddressModes)
{
    rhi::SamplerDesc desc;

    // Test Repeat
    desc.addressU = rhi::AddressMode::Repeat;
    desc.addressV = rhi::AddressMode::Repeat;
    desc.addressW = rhi::AddressMode::Repeat;
    auto handle1 = rhi.CreateSampler(desc);
    EXPECT_NE(handle1.id, 0xFFFFFFFFFFFFFFFF);

    // Test ClampToEdge
    desc.addressU = rhi::AddressMode::ClampToEdge;
    desc.addressV = rhi::AddressMode::ClampToEdge;
    desc.addressW = rhi::AddressMode::ClampToEdge;
    auto handle2 = rhi.CreateSampler(desc);
    EXPECT_NE(handle2.id, 0xFFFFFFFFFFFFFFFF);

    // Test MirrorRepeat
    desc.addressU = rhi::AddressMode::MirrorRepeat;
    desc.addressV = rhi::AddressMode::MirrorRepeat;
    desc.addressW = rhi::AddressMode::MirrorRepeat;
    auto handle3 = rhi.CreateSampler(desc);
    EXPECT_NE(handle3.id, 0xFFFFFFFFFFFFFFFF);
}

TEST_F(RHITest, CreateSamplerWithAnisotropy)
{
    rhi::SamplerDesc desc;
    desc.anisotropyEnabled = true;
    desc.maxAnisotropy = 16;

    auto handle = rhi.CreateSampler(desc);
    EXPECT_NE(handle.id, 0xFFFFFFFFFFFFFFFF);
}

TEST_F(RHITest, CreateSamplerWithComparison)
{
    rhi::SamplerDesc desc;
    desc.compareEnabled = true;
    desc.compareOp = rhi::CompareOp::Less;

    auto handle = rhi.CreateSampler(desc);
    EXPECT_NE(handle.id, 0xFFFFFFFFFFFFFFFF);
}

TEST_F(RHITest, CreateSamplerWithDifferentComparisons)
{
    rhi::SamplerDesc desc;
    desc.compareEnabled = true;

    rhi::CompareOp ops[] = {
        rhi::CompareOp::Never,
        rhi::CompareOp::Less,
        rhi::CompareOp::Equal,
        rhi::CompareOp::LessEqual,
        rhi::CompareOp::Greater,
        rhi::CompareOp::NotEqual,
        rhi::CompareOp::GreaterEqual,
        rhi::CompareOp::Always,
    };

    for (auto op : ops)
    {
        desc.compareOp = op;
        auto handle = rhi.CreateSampler(desc);
        EXPECT_NE(handle.id, 0xFFFFFFFFFFFFFFFF) << "Failed for CompareOp: " << static_cast<int>(op);
    }
}

TEST_F(RHITest, CreateSamplerWithBorderColors)
{
    rhi::SamplerDesc desc;
    desc.addressU = rhi::AddressMode::ClampToBorder;
    desc.addressV = rhi::AddressMode::ClampToBorder;
    desc.addressW = rhi::AddressMode::ClampToBorder;

    rhi::BorderColor colors[] = {
        rhi::BorderColor::TransparentBlack,
        rhi::BorderColor::OpaqueBlack,
        rhi::BorderColor::OpaqueWhite,
    };

    for (auto color : colors)
    {
        desc.borderColor = color;
        auto handle = rhi.CreateSampler(desc);
        EXPECT_NE(handle.id, 0xFFFFFFFFFFFFFFFF) << "Failed for BorderColor: " << static_cast<int>(color);
    }
}

TEST_F(RHITest, CreateSamplerWithLODBias)
{
    rhi::SamplerDesc desc;
    desc.mipLodBias = 2.0f;

    auto handle = rhi.CreateSampler(desc);
    EXPECT_NE(handle.id, 0xFFFFFFFFFFFFFFFF);
}

TEST_F(RHITest, CreateSamplerWithLODRange)
{
    rhi::SamplerDesc desc;
    desc.minLOD = 0.0f;
    desc.maxLOD = 10.0f;

    auto handle = rhi.CreateSampler(desc);
    EXPECT_NE(handle.id, 0xFFFFFFFFFFFFFFFF);
}

TEST_F(RHITest, DestroySampler)
{
    rhi::SamplerDesc desc;
    auto handle = rhi.CreateSampler(desc);
    EXPECT_NE(handle.id, 0xFFFFFFFFFFFFFFFF);

    // Should not crash
    rhi.DestroySampler(handle);
}

TEST_F(RHITest, CreateAndDestroyMultipleSamplers)
{
    rhi::SamplerDesc desc;

    auto handle1 = rhi.CreateSampler(desc);
    auto handle2 = rhi.CreateSampler(desc);
    auto handle3 = rhi.CreateSampler(desc);

    EXPECT_NE(handle1.id, 0xFFFFFFFFFFFFFFFF);
    EXPECT_NE(handle2.id, 0xFFFFFFFFFFFFFFFF);
    EXPECT_NE(handle3.id, 0xFFFFFFFFFFFFFFFF);

    // Destroy in reverse order
    rhi.DestroySampler(handle3);
    rhi.DestroySampler(handle2);
    rhi.DestroySampler(handle1);
}

// ============================================================================
// Buffer Tests
// ============================================================================

namespace
{
constexpr std::uint64_t InvalidHandle = 0xFFFFFFFFFFFFFFFF;

rhi::BufferDesc MakeBufferDesc(
    std::uint64_t size,
    rhi::MemoryUsage memoryUsage,
    rhi::BufferUsage usage = rhi::BufferUsage::CopySrc)
{
    rhi::BufferDesc desc;
    desc.size = size;
    desc.memoryUsage = memoryUsage;
    desc.usage = usage;
    return desc;
}
}

TEST_F(RHITest, CreateBuffer)
{
    auto handle = rhi.CreateBuffer(MakeBufferDesc(
        256, rhi::MemoryUsage::CpuToGpu));

    EXPECT_NE(handle.id, InvalidHandle);
    rhi.DestroyBuffer(handle);
}

TEST_F(RHITest, CreateMultipleBuffers)
{
    std::vector<rhi::BufferHandle> handles;
    for (int i = 0; i < 3; ++i) {
        auto handle = rhi.CreateBuffer(MakeBufferDesc(
            256, rhi::MemoryUsage::CpuToGpu));
        ASSERT_NE(handle.id, InvalidHandle);
        handles.push_back(handle);
    }

    EXPECT_NE(handles[0].id, handles[1].id);
    EXPECT_NE(handles[1].id, handles[2].id);

    for (auto handle : handles) {
        rhi.DestroyBuffer(handle);
    }
}

TEST_F(RHITest, RejectsZeroSizedBuffer)
{
    auto handle = rhi.CreateBuffer(MakeBufferDesc(
        0, rhi::MemoryUsage::CpuToGpu));

    EXPECT_EQ(handle.id, InvalidHandle);
}

TEST_F(RHITest, MapAndWriteCpuToGpuBuffer)
{
    constexpr std::size_t BufferSize = 64;
    constexpr std::uint32_t Value = 0x12345678;

    auto handle = rhi.CreateBuffer(MakeBufferDesc(
        BufferSize,
        rhi::MemoryUsage::CpuToGpu));
    ASSERT_NE(handle.id, InvalidHandle);

    auto* mapped = static_cast<std::uint32_t*>(rhi.MapBuffer(handle));
    ASSERT_NE(mapped, nullptr);
    mapped[0] = Value;
    rhi.UnmapBuffer(handle);

    std::uint32_t written = 0;
    rhi.WriteBuffer(handle, &Value, sizeof(Value));

    mapped = static_cast<std::uint32_t*>(rhi.MapBuffer(handle));
    ASSERT_NE(mapped, nullptr);
    std::memcpy(&written, mapped, sizeof(written));
    rhi.UnmapBuffer(handle);

    EXPECT_EQ(written, Value);
    rhi.DestroyBuffer(handle);
}

TEST_F(RHITest, WriteBufferWithOffset)
{
    constexpr std::size_t BufferSize = 64;
    constexpr std::size_t Offset = 16;
    constexpr std::array<std::uint8_t, 4> Data = { 1, 2, 3, 4 };

    auto handle = rhi.CreateBuffer(MakeBufferDesc(
        BufferSize, rhi::MemoryUsage::CpuToGpu));
    ASSERT_NE(handle.id, InvalidHandle);

    rhi.WriteBuffer(handle, Data.data(), Data.size(), Offset);

    auto* mapped = static_cast<std::uint8_t*>(rhi.MapBuffer(handle));
    ASSERT_NE(mapped, nullptr);
    EXPECT_EQ(std::memcmp(mapped + Offset, Data.data(), Data.size()), 0);
    rhi.UnmapBuffer(handle);
    rhi.DestroyBuffer(handle);
}

TEST_F(RHITest, RejectsOutOfBoundsBufferWrite)
{
    constexpr std::array<std::uint8_t, 8> Data = {};
    auto handle = rhi.CreateBuffer(MakeBufferDesc(
        16, rhi::MemoryUsage::CpuToGpu));
    ASSERT_NE(handle.id, InvalidHandle);

    // The operation should be rejected without crashing.
    rhi.WriteBuffer(handle, Data.data(), Data.size(), 12);

    rhi.DestroyBuffer(handle);
}

TEST_F(RHITest, GpuOnlyBufferCannotBeMapped)
{
    auto handle = rhi.CreateBuffer(MakeBufferDesc(
        256,
        rhi::MemoryUsage::GpuOnly,
        rhi::BufferUsage::Vertex));
    ASSERT_NE(handle.id, InvalidHandle);

    EXPECT_EQ(rhi.MapBuffer(handle), nullptr);
    rhi.UnmapBuffer(handle);
    rhi.DestroyBuffer(handle);
}

TEST_F(RHITest, DestroyBuffer)
{
    auto handle = rhi.CreateBuffer(MakeBufferDesc(
        256, rhi::MemoryUsage::CpuToGpu));
    ASSERT_NE(handle.id, InvalidHandle);

    rhi.DestroyBuffer(handle);
    EXPECT_EQ(rhi.MapBuffer(handle), nullptr);

    // Destroying the same handle again should not crash.
    rhi.DestroyBuffer(handle);
}

TEST_F(RHITest, CreateAndDestroyMultipleBuffers)
{
    std::vector<rhi::BufferHandle> handles;
    for (int i = 0; i < 16; ++i) {
        auto handle = rhi.CreateBuffer(MakeBufferDesc(
            128 + i, rhi::MemoryUsage::CpuToGpu));
        ASSERT_NE(handle.id, InvalidHandle);
        handles.push_back(handle);
    }

    for (auto it = handles.rbegin(); it != handles.rend(); ++it) {
        rhi.DestroyBuffer(*it);
    }
}

// ============================================================================
// Shader Compilation Tests
// ============================================================================

#if defined(RHI_VULKAN) || defined(RHI_DX12)
namespace
{
struct ShaderCompilation
{
    Microsoft::WRL::ComPtr<IDxcBlob> object;
};

ShaderCompilation CompileShaderForTest(
    RHIImpl& rhi,
    const char* sourceText,
    const wchar_t* profile,
    bool spirv = false)
{
    DxcBuffer source{};
    source.Ptr = sourceText;
    source.Size = std::strlen(sourceText);
    source.Encoding = DXC_CP_UTF8;

    std::vector<LPCWSTR> args = { L"-E", L"main", L"-T", profile };
#if defined(RHI_VULKAN)
    if (spirv) {
        args.push_back(L"-spirv");
        args.push_back(L"-fvk-use-dx-layout");
    }
#else
    (void)spirv;
#endif

    ShaderCompilation result;
    result.object = rhi.CompileShader(source, args);

    return result;
}

void ExpectShaderObjectValid(const ShaderCompilation& shader)
{
    ASSERT_NE(shader.object, nullptr);
    ASSERT_NE(shader.object->GetBufferPointer(), nullptr);
    EXPECT_GT(shader.object->GetBufferSize(), 4u);

    const auto* bytes = static_cast<const std::uint32_t*>(shader.object->GetBufferPointer());
#if defined(RHI_VULKAN)
    EXPECT_EQ(bytes[0], 0x07230203u) << "Expected SPIR-V magic number";
#else
    EXPECT_EQ(bytes[0], 0x43425844u) << "Expected DXIL container magic";
#endif
}
}

TEST_F(RHITest, CompileVertexShaderProducesValidBinary)
{
    constexpr char source[] =
        "struct VSInput { float3 position : POSITION; };\n"
        "struct VSOutput { float4 position : SV_Position; };\n"
        "VSOutput main(VSInput input) { VSOutput output; "
        "output.position = float4(input.position, 1.0); return output; }\n";

    auto shader = CompileShaderForTest(rhi, source, L"vs_6_0", true);
    ExpectShaderObjectValid(shader);
}

TEST_F(RHITest, CompilePixelShaderProducesValidBinary)
{
    constexpr char source[] =
        "float4 main() : SV_Target { return float4(1, 0, 0, 1); }\n";

    auto shader = CompileShaderForTest(rhi, source, L"ps_6_0", true);
    ExpectShaderObjectValid(shader);
}

TEST_F(RHITest, CompileComputeShaderProducesValidBinary)
{
    constexpr char source[] =
        "[numthreads(8, 4, 2)] void main(uint3 id : SV_DispatchThreadID) { }\n";

    auto shader = CompileShaderForTest(rhi, source, L"cs_6_0", true);
    ExpectShaderObjectValid(shader);
}

TEST_F(RHITest, RejectInvalidShaderSource)
{
    constexpr char source[] =
        "float4 main() : SV_Target { return does_not_exist; }\n";

    auto shader = CompileShaderForTest(rhi, source, L"ps_6_0", true);
    EXPECT_EQ(shader.object, nullptr);
}
#endif

// ============================================================================
// Texture Tests
// ============================================================================

namespace
{
constexpr std::uint64_t InvalidTextureHandle = 0xFFFFFFFF;

#if defined(RHI_VULKAN) || defined(RHI_WEBGPU)
#define RHI_HAS_TEXTURE_VIEWS 1
#endif

rhi::TextureDesc MakeTextureDesc(
    std::uint32_t width = 4,
    std::uint32_t height = 4,
    rhi::TextureFormat format = rhi::TextureFormat::RGBA8Unorm,
    rhi::TextureUsage usage = rhi::TextureUsage::Sampled |
                               rhi::TextureUsage::CopyDst)
{
    rhi::TextureDesc desc;
    desc.width = width;
    desc.height = height;
    desc.format = format;
    desc.usage = usage;
    return desc;
}
}

TEST_F(RHITest, CreateTexture)
{
    auto handle = rhi.CreateTexture(MakeTextureDesc());

    EXPECT_NE(handle.id, InvalidTextureHandle);
    rhi.DestroyTexture(handle);
}

TEST_F(RHITest, CreateMultipleTextures)
{
    std::vector<rhi::TextureHandle> handles;
    for (int i = 0; i < 3; ++i) {
        auto handle = rhi.CreateTexture(MakeTextureDesc(4 + i, 4 + i));
        ASSERT_NE(handle.id, InvalidTextureHandle);
        handles.push_back(handle);
    }

    EXPECT_NE(handles[0].id, handles[1].id);
    EXPECT_NE(handles[1].id, handles[2].id);

    for (auto handle : handles) {
        rhi.DestroyTexture(handle);
    }
}

TEST_F(RHITest, CreateTextureWithDifferentDimensionsAndFormats)
{
    struct TextureCase {
        rhi::TextureDimension dimension;
        std::uint32_t width;
        std::uint32_t height;
        std::uint32_t depth;
        rhi::TextureFormat format;
    };

    const TextureCase cases[] = {
        { rhi::TextureDimension::Tex1D, 16, 1, 1, rhi::TextureFormat::R8Unorm },
        { rhi::TextureDimension::Tex2D, 8, 8, 1, rhi::TextureFormat::RGBA8Unorm },
        { rhi::TextureDimension::Tex3D, 4, 4, 4, rhi::TextureFormat::RGBA16Float },
    };

    for (const auto& testCase : cases) {
        auto desc = MakeTextureDesc(
            testCase.width,
            testCase.height,
            testCase.format,
            rhi::TextureUsage::Sampled | rhi::TextureUsage::Storage |
                rhi::TextureUsage::CopySrc | rhi::TextureUsage::CopyDst);
        desc.dimension = testCase.dimension;
        desc.depth = testCase.depth;

        auto handle = rhi.CreateTexture(desc);
        ASSERT_NE(handle.id, InvalidTextureHandle)
            << "Failed for texture dimension/format case";
        rhi.DestroyTexture(handle);
    }
}

TEST_F(RHITest, CreateTextureWithMipLevelsAndSampleCount)
{
    auto desc = MakeTextureDesc(
        16,
        16,
        rhi::TextureFormat::RGBA8Unorm,
        rhi::TextureUsage::Sampled | rhi::TextureUsage::RenderTarget);
    desc.mipLevels = 4;
    desc.sampleCount = rhi::SampleCount::x1;

    auto handle = rhi.CreateTexture(desc);
    ASSERT_NE(handle.id, InvalidTextureHandle);

    rhi.DestroyTexture(handle);
}

#if defined(RHI_HAS_TEXTURE_VIEWS)
TEST_F(RHITest, CreateTextureView)
{
    auto texture = rhi.CreateTexture(MakeTextureDesc());
    ASSERT_NE(texture.id, InvalidTextureHandle);

    rhi::TextureViewDesc viewDesc;
    auto view = rhi.CreateTextureView(viewDesc, texture);

    EXPECT_NE(view.id, InvalidTextureHandle);
    rhi.DestroyTextureView(view);
    rhi.DestroyTexture(texture);
}

TEST_F(RHITest, CreateTextureViewsForMipLevelsAndArrayLayers)
{
    auto textureDesc = MakeTextureDesc();
    textureDesc.mipLevels = 3;
    textureDesc.arrayLayers = 2;

    auto texture = rhi.CreateTexture(textureDesc);
    ASSERT_NE(texture.id, InvalidTextureHandle);

    rhi::TextureViewDesc viewDesc;
    viewDesc.baseMipLevel = 1;
    viewDesc.baseArrayLayer = 1;
    viewDesc.mipLevels = 1;
    viewDesc.arrayLayers = 1;

    auto view = rhi.CreateTextureView(viewDesc, texture);
    ASSERT_NE(view.id, InvalidTextureHandle);

    rhi.DestroyTextureView(view);
    rhi.DestroyTexture(texture);
}

TEST_F(RHITest, CreateAndDestroyMultipleTextureViews)
{
    auto texture = rhi.CreateTexture(MakeTextureDesc());
    ASSERT_NE(texture.id, InvalidTextureHandle);

    rhi::TextureViewDesc viewDesc;
    auto view1 = rhi.CreateTextureView(viewDesc, texture);
    auto view2 = rhi.CreateTextureView(viewDesc, texture);
    ASSERT_NE(view1.id, InvalidTextureHandle);
    ASSERT_NE(view2.id, InvalidTextureHandle);
    EXPECT_NE(view1.id, view2.id);

    rhi.DestroyTextureView(view2);
    rhi.DestroyTextureView(view1);
    rhi.DestroyTexture(texture);
}

TEST_F(RHITest, DestroyTextureAndTextureViewAreIdempotent)
{
    auto texture = rhi.CreateTexture(MakeTextureDesc());
    ASSERT_NE(texture.id, InvalidTextureHandle);

    rhi::TextureViewDesc viewDesc;
    auto view = rhi.CreateTextureView(viewDesc, texture);
    ASSERT_NE(view.id, InvalidTextureHandle);

    rhi.DestroyTextureView(view);
    rhi.DestroyTextureView(view);
    rhi.DestroyTexture(texture);
    rhi.DestroyTexture(texture);
}

#endif

TEST_F(RHITest, WriteTextureAcceptsTextureData)
{
    auto texture = rhi.CreateTexture(MakeTextureDesc(
        4,
        4,
        rhi::TextureFormat::RGBA8Unorm,
        rhi::TextureUsage::CopyDst | rhi::TextureUsage::Sampled));
    ASSERT_NE(texture.id, InvalidTextureHandle);

    constexpr std::array<std::uint8_t, 64> pixels{};
    rhi::TextureWriteDesc writeDesc;
    writeDesc.width = 4;
    writeDesc.height = 4;
    writeDesc.bytesPerRow = 16;
    writeDesc.rowsPerImage = 4;

    // Upload is not implemented by every backend yet; this verifies that the
    // public call is safe for a valid texture.
    rhi.WriteTexture(texture, writeDesc, pixels.data());
    rhi.DestroyTexture(texture);
}

// ============================================================================
// Stress Tests
// ============================================================================

TEST_F(RHITest, CreateManySimpleSamplers)
{
    rhi::SamplerDesc desc;
    std::vector<rhi::SamplerHandle> handles;

    // Create 100 samplers
    for (int i = 0; i < 100; ++i)
    {
        auto handle = rhi.CreateSampler(desc);
        EXPECT_NE(handle.id, 0xFFFFFFFFFFFFFFFF) << "Failed at sampler " << i;
        handles.push_back(handle);
    }

    // All handles should be unique
    for (size_t i = 0; i < handles.size(); ++i)
    {
        for (size_t j = i + 1; j < handles.size(); ++j)
        {
            EXPECT_NE(handles[i].id, handles[j].id);
        }
    }
}

TEST_F(RHITest, CreateComplexSamplerCombinations)
{
    // Test various combinations of parameters
    rhi::SamplerDesc desc;

    rhi::Filter filters[] = { rhi::Filter::Nearest, rhi::Filter::Linear };
    rhi::MipmapFilter mipFilters[] = { rhi::MipmapFilter::Nearest, rhi::MipmapFilter::Linear };
    rhi::AddressMode addressModes[] = {
        rhi::AddressMode::Repeat,
        rhi::AddressMode::ClampToEdge,
        rhi::AddressMode::MirrorRepeat,
    };

    int count = 0;
    for (auto minF : filters)
    {
        for (auto magF : filters)
        {
            for (auto mipF : mipFilters)
            {
                for (auto addr : addressModes)
                {
                    desc.minFilter = minF;
                    desc.magFilter = magF;
                    desc.mipFilter = mipF;
                    desc.addressU = addr;
                    desc.addressV = addr;
                    desc.addressW = addr;

                    auto handle = rhi.CreateSampler(desc);
                    EXPECT_NE(handle.id, 0xFFFFFFFFFFFFFFFF) << "Failed at combination " << count;
                    count++;
                }
            }
        }
    }
}
