#include "rhi/webgpu/wgpurhi.h"

#include <string>


wgpu::ShaderModule MakeShaderModule(
    const wgpu::Device& device,
    const rhi::ShaderModule& module)
{
    if (module.bytecode.empty()) {
        return nullptr;
    }

    wgpu::ShaderSourceWGSL wgsl{};
    wgsl.code = wgpu::StringView(
        reinterpret_cast<const char*>(module.bytecode.data()),
        module.bytecode.size());

    wgpu::ShaderModuleDescriptor modDesc{};
    modDesc.nextInChain = &wgsl;
    return device.CreateShaderModule(&modDesc);
}


rhi::GraphicsPipelineHandle WebGPURHI::CreateGraphicsPipeline(
    const rhi::GraphicsPipelineDesc& desc)
{
    rhi::GraphicsPipelineHandle handle;

    // WebGPU core has no wireframe fill mode and only supports 1x/4x MSAA.
    if (desc.fillMode != rhi::FillMode::Solid) {
        printf("[RHI] Wireframe fill mode is unsupported in WebGPU\n");
        return handle;
    }
    if (desc.sampleCount != rhi::SampleCount::x1 &&
        desc.sampleCount != rhi::SampleCount::x4) {
        printf("[RHI] Only 1x and 4x MSAA are supported in WebGPU\n");
        return handle;
    }
    if (desc.colorTargets.empty()) {
        return handle;
    }
    if (desc.layout.id >= m_PipelineLayouts.size() ||
        !m_PipelineLayouts[desc.layout.id]) {
        return handle;
    }
    if (desc.vertexShader.id >= m_ShaderModules.size() ||
        desc.fragmentShader.id >= m_ShaderModules.size()) {
        return handle;
    }

    const rhi::ShaderModule& vsModule = m_ShaderModules[desc.vertexShader.id];
    const rhi::ShaderModule& fsModule = m_ShaderModules[desc.fragmentShader.id];
    if (vsModule.bytecode.empty() || fsModule.bytecode.empty()) {
        return handle;
    }

    wgpu::ShaderModule vs = MakeShaderModule(m_Device, vsModule);
    wgpu::ShaderModule fs = MakeShaderModule(m_Device, fsModule);
    if (!vs || !fs) {
        return handle;
    }

    std::vector<std::vector<wgpu::VertexAttribute>> wgpuAttributes(desc.vertexBuffers.size());
    std::vector<wgpu::VertexBufferLayout> wgpuBuffers;
    wgpuBuffers.reserve(desc.vertexBuffers.size());
    for (std::size_t i = 0; i < desc.vertexBuffers.size(); ++i) {
        const auto& buffer = desc.vertexBuffers[i];
        auto& attributes = wgpuAttributes[i];
        attributes.reserve(buffer.attributes.size());
        for (const auto& attr : buffer.attributes) {
            wgpu::VertexAttribute wgpuAttr{};
            wgpuAttr.format = toWebGPU(attr.format);
            wgpuAttr.offset = attr.offset;
            wgpuAttr.shaderLocation = attr.location;
            attributes.push_back(wgpuAttr);
        }

        wgpu::VertexBufferLayout wgpuBuffer{};
        wgpuBuffer.arrayStride = buffer.stride;
        wgpuBuffer.stepMode = wgpu::VertexStepMode::Vertex;
        wgpuBuffer.attributeCount = attributes.size();
        wgpuBuffer.attributes = attributes.data();
        wgpuBuffers.push_back(wgpuBuffer);
    }

    std::vector<wgpu::BlendState> wgpuBlends(desc.colorTargets.size());
    std::vector<wgpu::ColorTargetState> wgpuTargets;
    wgpuTargets.reserve(desc.colorTargets.size());
    for (std::size_t i = 0; i < desc.colorTargets.size(); ++i) {
        const auto& target = desc.colorTargets[i];
        const wgpu::TextureFormat format = toWebGPU(target.format);
        if (format == wgpu::TextureFormat::Undefined) {
            return handle;
        }

        wgpu::ColorTargetState wgpuTarget{};
        wgpuTarget.format = format;
        wgpuTarget.writeMask = toWebGPU(target.blend.writeMask);
        if (target.blend.enabled) {
            auto& blend = wgpuBlends[i];
            blend.color.operation = toWebGPU(target.blend.colorOp);
            blend.color.srcFactor = toWebGPU(target.blend.srcColor);
            blend.color.dstFactor = toWebGPU(target.blend.dstColor);
            blend.alpha.operation = toWebGPU(target.blend.alphaOp);
            blend.alpha.srcFactor = toWebGPU(target.blend.srcAlpha);
            blend.alpha.dstFactor = toWebGPU(target.blend.dstAlpha);
            if (blend.color.operation == wgpu::BlendOperation::Undefined ||
                blend.alpha.operation == wgpu::BlendOperation::Undefined) {
                return handle;
            }
            wgpuTarget.blend = &blend;
        }
        wgpuTargets.push_back(wgpuTarget);
    }

    wgpu::RenderPipelineDescriptor pipelineDesc{};
    pipelineDesc.layout = m_PipelineLayouts[desc.layout.id];

    wgpu::VertexState vertex{};
    vertex.module = vs;
    vertex.entryPoint = wgpu::StringView(
        vsModule.entryPoint.data(), vsModule.entryPoint.size());
    vertex.bufferCount = wgpuBuffers.size();
    vertex.buffers = wgpuBuffers.data();
    pipelineDesc.vertex = vertex;

    pipelineDesc.primitive.topology = toWebGPU(desc.topology);
    pipelineDesc.primitive.frontFace = toWebGPU(desc.frontFace);
    pipelineDesc.primitive.cullMode = toWebGPU(desc.cullMode);
    if (pipelineDesc.primitive.topology == wgpu::PrimitiveTopology::Undefined ||
        pipelineDesc.primitive.cullMode == wgpu::CullMode::Undefined) {
        return handle;
    }

    wgpu::DepthStencilState depthStencil{};
    if (desc.depthStencil.format != rhi::TextureFormat::Unknown) {
        const wgpu::TextureFormat format = toWebGPU(desc.depthStencil.format);
        if (format == wgpu::TextureFormat::Undefined) {
            return handle;
        }
        depthStencil.format = format;
        depthStencil.depthWriteEnabled = desc.depthStencil.depthWriteEnabled
            ? wgpu::OptionalBool::True
            : wgpu::OptionalBool::False;
        depthStencil.depthCompare = desc.depthStencil.depthTestEnabled
            ? toWebGPU(desc.depthStencil.depthFunc)
            : wgpu::CompareFunction::Always;
        depthStencil.stencilFront.compare = wgpu::CompareFunction::Always;
        depthStencil.stencilBack.compare = wgpu::CompareFunction::Always;
        depthStencil.stencilReadMask = 0;
        depthStencil.stencilWriteMask = 0;
        pipelineDesc.depthStencil = &depthStencil;
    }

    pipelineDesc.multisample.count = toWebGPU(desc.sampleCount);
    pipelineDesc.multisample.mask = 0xFFFFFFFF;

    wgpu::FragmentState fragment{};
    fragment.module = fs;
    fragment.entryPoint = wgpu::StringView(
        fsModule.entryPoint.data(), fsModule.entryPoint.size());
    fragment.targetCount = wgpuTargets.size();
    fragment.targets = wgpuTargets.data();
    pipelineDesc.fragment = &fragment;

    wgpu::RenderPipeline pipeline = m_Device.CreateRenderPipeline(&pipelineDesc);
    if (!pipeline) {
        return handle;
    }

    handle.id = m_Pipelines.size();
    handle.desc = desc;
    m_Pipelines.push_back(pipeline);
    return handle;
}

rhi::PipelineLayoutHandle WebGPURHI::CreatePipelineLayout(
    const rhi::PipelineLayoutDesc& desc)
{
    rhi::PipelineLayoutHandle handle;

    if (desc.layouts.empty()) {
        return handle;
    }

    std::vector<wgpu::BindGroupLayout> layouts;
    layouts.reserve(desc.layouts.size());
    for (const auto& layout : desc.layouts) {
        if (layout.id >= m_BindGroupLayouts.size() ||
            !m_BindGroupLayouts[layout.id]) {
            return rhi::PipelineLayoutHandle{};
        }
        layouts.push_back(m_BindGroupLayouts[layout.id]);
    }

    wgpu::PipelineLayoutDescriptor layoutDesc{};
    layoutDesc.bindGroupLayoutCount = layouts.size();
    layoutDesc.bindGroupLayouts = layouts.data();

    wgpu::PipelineLayout pipelineLayout = m_Device.CreatePipelineLayout(&layoutDesc);
    if (!pipelineLayout) {
        return rhi::PipelineLayoutHandle{};
    }

    handle.id = m_PipelineLayouts.size();
    handle.desc = desc;
    m_PipelineLayouts.push_back(pipelineLayout);
    return handle;
}
