#include <gtest/gtest.h>
#include "rhi/IRHI.h"
#include <array>
#include <cstdint>
#include <cstring>
#include <vector>


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
        rhi::MemoryUsage::CpuToGpu,
        rhi::BufferUsage::Vertex | rhi::BufferUsage::CopySrc));
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
