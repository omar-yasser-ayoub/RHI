#include <gtest/gtest.h>
#include "rhi/IRHI.h"
#if defined(RHI_WEBGPU) && defined(_WIN32)
    #include <Windows.h>
#endif
#include <array>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <string>
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


#if defined(RHI_NEVER)
namespace
{
class TemporaryShaderFiles
{
public:
    TemporaryShaderFiles()
    {
        const auto base = std::filesystem::temp_directory_path() /
            ("rhi_shader_test_" + std::to_string(std::rand()));
        m_Hlsl = base;
        m_Hlsl += ".hlsl";
        m_Spirv = base;
        m_Spirv += ".spv";
        m_Wgsl = base;
        m_Wgsl += ".wgsl";
    }

    ~TemporaryShaderFiles()
    {
        std::error_code error;
        std::filesystem::remove(m_Hlsl, error);
        std::filesystem::remove(m_Spirv, error);
        std::filesystem::remove(m_Wgsl, error);
    }

    std::filesystem::path m_Hlsl;
    std::filesystem::path m_Spirv;
    std::filesystem::path m_Wgsl;
};

#if defined(_WIN32)
bool RunTool(
    const char* executable,
    const std::string& arguments)
{
    std::string commandLine = "\"" + std::string(executable) + "\" " + arguments;
    std::vector<char> mutableCommandLine(commandLine.begin(), commandLine.end());
    mutableCommandLine.push_back('\0');

    STARTUPINFOA startupInfo{};
    startupInfo.cb = sizeof(startupInfo);
    PROCESS_INFORMATION processInfo{};

    if (!CreateProcessA(
            nullptr,
            mutableCommandLine.data(),
            nullptr,
            nullptr,
            FALSE,
            CREATE_NO_WINDOW,
            nullptr,
            nullptr,
            &startupInfo,
            &processInfo)) {
        return false;
    }

    WaitForSingleObject(processInfo.hProcess, INFINITE);
    DWORD exitCode = 1;
    GetExitCodeProcess(processInfo.hProcess, &exitCode);
    CloseHandle(processInfo.hThread);
    CloseHandle(processInfo.hProcess);
    return exitCode == 0;
}
#endif

bool RunShaderConversionTest(
    const char* source,
    const char* profile,
    const char* expectedWgslMarker)
{
    TemporaryShaderFiles files;
    {
        std::ofstream output(files.m_Hlsl, std::ios::binary);
        if (!output.is_open()) {
            std::fprintf(stderr, "Failed to open HLSL file: %s\n", files.m_Hlsl.string().c_str());
            return false;
        }
        output << source;
    }

    const std::string dxcArguments =
        "-spirv -E main -T " + std::string(profile) +
        " -fvk-use-dx-layout -Fo \"" + files.m_Spirv.string() +
        "\" \"" + files.m_Hlsl.string() + "\"";
#if defined(_WIN32)
    if (!RunTool(RHI_DXC_EXE, dxcArguments)) {
        std::fprintf(stderr, "DXC failed for %s\n", files.m_Hlsl.string().c_str());
        return false;
    }
#else
    return false;
#endif
    if (!std::filesystem::exists(files.m_Spirv)) {
        std::fprintf(stderr, "DXC did not produce SPIR-V: %s\n", files.m_Spirv.string().c_str());
        return false;
    }

    std::ifstream spirv(files.m_Spirv, std::ios::binary);
    if (!spirv.is_open()) {
        std::fprintf(stderr, "Failed to open SPIR-V: %s\n", files.m_Spirv.string().c_str());
        return false;
    }
    std::uint32_t magic = 0;
    spirv.read(reinterpret_cast<char*>(&magic), sizeof(magic));
    if (magic != 0x07230203u) {
        std::fprintf(stderr, "Invalid SPIR-V magic in %s\n", files.m_Spirv.string().c_str());
        return false;
    }

    const std::string tintArguments =
        "--format wgsl -o \"" + files.m_Wgsl.string() +
        "\" \"" + files.m_Spirv.string() + "\"";
#if defined(_WIN32)
    if (!RunTool(RHI_TINT_EXE, tintArguments)) {
        std::fprintf(stderr, "Tint failed for %s\n", files.m_Spirv.string().c_str());
        return false;
    }
#else
    return false;
#endif
    if (!std::filesystem::exists(files.m_Wgsl)) {
        std::fprintf(stderr, "Tint did not produce WGSL: %s\n", files.m_Wgsl.string().c_str());
        return false;
    }

    std::ifstream wgsl(files.m_Wgsl);
    if (!wgsl.is_open()) {
        std::fprintf(stderr, "Failed to open WGSL: %s\n", files.m_Wgsl.string().c_str());
        return false;
    }
    std::ostringstream output;
    output << wgsl.rdbuf();
    const std::string wgslText = output.str();
    if (wgslText.empty()) {
        std::fprintf(stderr, "Tint produced empty WGSL\n");
        return false;
    }
    if (wgslText.find(expectedWgslMarker) == std::string::npos) {
        std::fprintf(stderr, "WGSL is missing marker %s\n", expectedWgslMarker);
        return false;
    }
    return true;
}
}

TEST(RHIShaderToolTests, ConvertVertexHlslToSpirvAndWgsl)
{
    EXPECT_TRUE(RunShaderConversionTest(
        "struct VSInput { float3 position : POSITION; };\n"
        "float4 main(VSInput input) : SV_Position {\n"
        "    return float4(input.position, 1.0);\n"
        "}\n",
        "vs_6_0",
        "@vertex"));
}

TEST(RHIShaderToolTests, ConvertPixelHlslToSpirvAndWgsl)
{
    EXPECT_TRUE(RunShaderConversionTest(
        "float4 main() : SV_Target { return float4(1, 0, 0, 1); }\n",
        "ps_6_0",
        "@fragment"));
}

TEST(RHIShaderToolTests, ConvertComputeHlslToSpirvAndWgsl)
{
    EXPECT_TRUE(RunShaderConversionTest(
        "[numthreads(8, 4, 2)]\n"
        "void main(uint3 id : SV_DispatchThreadID) { }\n",
        "cs_6_0",
        "@compute"));
}
#endif

#if defined(RHI_DX12)
// ============================================================================
// DX12 Shader Module Tests
// ============================================================================

namespace
{
std::filesystem::path ShaderPath(const char* filename)
{
#ifdef RHI_SHADER_DIR
    return std::filesystem::path(RHI_SHADER_DIR) / filename;
#else
    return std::filesystem::path("src/shaders") / filename;
#endif
}

rhi::ShaderSource MakeShaderSource(
    const char* filename,
    rhi::ShaderStageFlags stage)
{
    rhi::ShaderSource source;
    source.path = ShaderPath(filename);
    source.stage = stage;
    source.entryPoint = "main";
    return source;
}
}

TEST_F(RHITest, CompileVertexShaderModuleFromShaderAsset)
{
    const auto source = MakeShaderSource(
        "basic.vert.hlsl",
        rhi::ShaderStageFlags::Vertex);
    ASSERT_TRUE(std::filesystem::exists(source.path)) << source.path.string();

    const auto module = rhi.CompileShader(source);

    ASSERT_FALSE(module.bytecode.empty());
    ASSERT_GE(module.bytecode.size(), 4u);
    EXPECT_EQ(module.bytecode[0], static_cast<std::uint8_t>('D'));
    EXPECT_EQ(module.bytecode[1], static_cast<std::uint8_t>('X'));
    EXPECT_EQ(module.bytecode[2], static_cast<std::uint8_t>('B'));
    EXPECT_EQ(module.bytecode[3], static_cast<std::uint8_t>('C'));

    ASSERT_EQ(module.reflection.bindings.size(), 0u);
    ASSERT_EQ(module.reflection.vertexInputs.size(), 2u);

    const auto& position = module.reflection.vertexInputs[0];
    EXPECT_EQ(position.location, 0u);
    EXPECT_EQ(position.format, rhi::VertexFormat::Float32x3);
    EXPECT_EQ(position.stride, 0u);
    EXPECT_EQ(position.offset, 0u);

    const auto& uv = module.reflection.vertexInputs[1];
    EXPECT_EQ(uv.location, 1u);
    EXPECT_EQ(uv.format, rhi::VertexFormat::Float32x2);
    EXPECT_EQ(uv.stride, 0u);
    EXPECT_EQ(uv.offset, 0u);
}

TEST_F(RHITest, CompileFragmentShaderModuleFromShaderAsset)
{
    const auto source = MakeShaderSource(
        "basic.frag.hlsl",
        rhi::ShaderStageFlags::Fragment);
    ASSERT_TRUE(std::filesystem::exists(source.path)) << source.path.string();

    const auto module = rhi.CompileShader(source);

    ASSERT_FALSE(module.bytecode.empty());
    ASSERT_GE(module.bytecode.size(), 4u);
    EXPECT_EQ(module.bytecode[0], static_cast<std::uint8_t>('D'));
    EXPECT_EQ(module.bytecode[1], static_cast<std::uint8_t>('X'));
    EXPECT_EQ(module.bytecode[2], static_cast<std::uint8_t>('B'));
    EXPECT_EQ(module.bytecode[3], static_cast<std::uint8_t>('C'));
    EXPECT_TRUE(module.reflection.bindings.empty());
    EXPECT_TRUE(module.reflection.vertexInputs.empty());
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
