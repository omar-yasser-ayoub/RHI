#include <iostream>

#include "rhi/rhi.h"

#define GLFW_INCLUDE_NONE
#include <third_party/glfw/include/GLFW/glfw3.h>
#if defined(_WIN32)
#define GLFW_EXPOSE_NATIVE_WIN32
#include <third_party/glfw/include/GLFW/glfw3native.h>
#include <Windows.h>
#endif

constexpr std::uint32_t kWidth = 1280;
constexpr std::uint32_t kHeight = 720;

struct Vertex {
    float pos[3];
    float uv[2];
};

rhi::SwapchainHandle RecreateSwapchain(RHI& rhi, rhi::SwapchainHandle old, rhi::SurfaceHandle surface)
{
    rhi.WaitForIdle();
    rhi.DestroySwapchain(old);

    rhi::SwapchainDesc desc;
    desc.surface = surface;
    desc.width = kWidth;
    desc.height = kHeight;
    desc.format = rhi::TextureFormat::BGRA8Unorm;
    desc.presentMode = rhi::PresentMode::Fifo;
    desc.imageCount = 3;
    return rhi.CreateSwapchain(desc);
}

int main()
{
    RHI rhi;

    if (!rhi.Initialize()) {
        std::cerr << "Failed to initialize RHI.\n";
        return 1;
    }

    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW.\n";
        return 1;
    }
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    GLFWwindow* window = glfwCreateWindow(
        static_cast<int>(kWidth), static_cast<int>(kHeight),
        "hello triangle", nullptr, nullptr);
    if (window == nullptr) {
        std::cerr << "Failed to create a window.\n";
        glfwTerminate();
        return 1;
    }

    rhi::SurfaceDesc surfaceDesc;
#if defined(_WIN32)
    surfaceDesc.window.type = rhi::NativeWindowType::Win32;
    surfaceDesc.window.handle = glfwGetWin32Window(window);
    surfaceDesc.window.display = GetModuleHandle(nullptr);
#else
    std::cerr << "Native windowing is only implemented for Win32.\n";
    return 1;
#endif
    const rhi::SurfaceHandle surface = rhi.CreateSurface(surfaceDesc);
    if (surface.id == 0xFFFFFFFFFFFFFFFFull) {
        std::cerr << "Failed to create a surface.\n";
        return 1;
    }

    rhi::SwapchainDesc swapchainDesc;
    swapchainDesc.surface = surface;
    swapchainDesc.width = kWidth;
    swapchainDesc.height = kHeight;
    swapchainDesc.format = rhi::TextureFormat::BGRA8Unorm;
    swapchainDesc.presentMode = rhi::PresentMode::Fifo;
    swapchainDesc.imageCount = 3;
    rhi::SwapchainHandle swapchain = rhi.CreateSwapchain(swapchainDesc);
    if (swapchain.id == 0xFFFFFFFFFFFFFFFFull) {
        std::cerr << "Failed to create a swapchain.\n";
        return 1;
    }

    rhi::ShaderSource vsSource;
    vsSource.path = "shaders/basic.vert.hlsl";
    vsSource.stage = rhi::ShaderStageFlags::Vertex;
    vsSource.entryPoint = "main";
    const rhi::ShaderModuleHandle vs = rhi.CompileShader(vsSource);

    rhi::ShaderSource fsSource;
    fsSource.path = "shaders/basic.frag.hlsl";
    fsSource.stage = rhi::ShaderStageFlags::Fragment;
    fsSource.entryPoint = "main";
    const rhi::ShaderModuleHandle fs = rhi.CompileShader(fsSource);

    const auto vsModule = rhi.GetShaderModule(vs);
    const auto fsModule = rhi.GetShaderModule(fs);
    if (!vsModule.has_value() || !fsModule.has_value()) {
        std::cerr << "Failed to compile shaders.\n";
        return 1;
    }

    rhi::PipelineLayoutDesc layoutDesc;
    const std::vector<rhi::ShaderReflection> reflections = {
        vsModule->reflection, fsModule->reflection
    };
    for (const auto& bindGroupDesc : rhi::BuildBindGroupLayoutDescs(reflections)) {
        const rhi::BindGroupLayoutHandle bindGroupLayout =
            rhi.CreateBindGroupLayout(bindGroupDesc);
        if (bindGroupLayout.id == 0xFFFFFFFFFFFFFFFFull) {
            std::cerr << "Failed to create a bind group layout.\n";
            return 1;
        }
        layoutDesc.layouts.push_back(bindGroupLayout);
    }
    const rhi::PipelineLayoutHandle layout = rhi.CreatePipelineLayout(layoutDesc);

    rhi::GraphicsPipelineDesc pipelineDesc;
    pipelineDesc.layout = layout;
    pipelineDesc.vertexShader = vs;
    pipelineDesc.fragmentShader = fs;
    rhi::VertexBufferLayout bufferLayout;
    bufferLayout.stride = 20;
    bufferLayout.attributes.push_back({ 0, 0, rhi::VertexFormat::Float32x3, 0 });
    bufferLayout.attributes.push_back({ 0, 1, rhi::VertexFormat::Float32x2, 12 });
    pipelineDesc.vertexBuffers.push_back(bufferLayout);
    rhi::ColorTarget target;
    target.format = rhi::TextureFormat::BGRA8Unorm;
    pipelineDesc.colorTargets.push_back(target);
    const rhi::GraphicsPipelineHandle pipeline = rhi.CreateGraphicsPipeline(pipelineDesc);
    if (pipeline.id == 0xFFFFFFFFFFFFFFFFull) {
        std::cerr << "Failed to create a graphics pipeline.\n";
        return 1;
    }

    const Vertex triangle[3] = {
        { { 0.0f, 0.5f, 0.0f }, { 0.5f, 0.0f } },
        { { -0.5f, -0.5f, 0.0f }, { 0.0f, 1.0f } },
        { { 0.5f, -0.5f, 0.0f }, { 1.0f, 1.0f } },
    };

    rhi::BufferDesc vertexDesc;
    vertexDesc.size = sizeof(triangle);
    vertexDesc.memoryUsage = rhi::MemoryUsage::CpuToGpu;
    vertexDesc.usage = rhi::BufferUsage::Vertex;
    const rhi::BufferHandle vertexBuffer = rhi.CreateBuffer(vertexDesc);
    rhi.WriteBuffer(vertexBuffer, triangle, sizeof(triangle));

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        const rhi::AcquireResult acquired = rhi.AcquireNextImage(swapchain);
        if (acquired.status == rhi::AcquireStatus::OutOfDate) {
            swapchain = RecreateSwapchain(rhi, swapchain, surface);
            continue;
        }
        if (acquired.status != rhi::AcquireStatus::Success &&
            acquired.status != rhi::AcquireStatus::Suboptimal) {
            continue;
        }

        const std::uint32_t width = swapchain.desc.width;
        const std::uint32_t height = swapchain.desc.height;

        rhi::RenderPassDesc pass{};
        rhi::RenderPassColorAttachment color{};
        color.useSwapchain = true;
        color.clearColor[0] = 0.1f;
        color.clearColor[1] = 0.1f;
        color.clearColor[2] = 0.1f;
        color.clearColor[3] = 1.0f;
        pass.colors.push_back(color);
        pass.swapchain = swapchain;
        pass.imageIndex = acquired.imageIndex;

        rhi::CommandListHandle cmd = rhi.BeginCommandList();
        rhi.BeginRenderPass(cmd, pass);
        rhi.SetPipeline(cmd, pipeline);
        rhi.SetVertexBuffer(cmd, 0, vertexBuffer, 0);
        rhi.SetViewport(cmd, 0, 0, static_cast<float>(width), static_cast<float>(height));
        rhi.SetScissor(cmd, 0, 0, width, height);
        rhi.Draw(cmd, 3, 1, 0, 0);
        rhi.EndPass(cmd);
        rhi.Submit({&cmd, 1});
        rhi.Present(swapchain);
    }

    rhi.WaitForIdle();
    rhi.DestroyBuffer(vertexBuffer);
    rhi.DestroyPipeline(pipeline);
    rhi.DestroyPipelineLayout(layout);
    rhi.DestroyShaderModule(vs);
    rhi.DestroyShaderModule(fs);
    rhi.DestroySwapchain(swapchain);
    rhi.DestroySurface(surface);
    glfwDestroyWindow(window);
    glfwTerminate();
    rhi.Shutdown();
    return 0;
}
