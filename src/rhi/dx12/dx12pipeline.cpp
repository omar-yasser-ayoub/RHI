#include "rhi/dx12/dx12rhi.h"

rhi::PipelineLayoutHandle DX12RHI::CreatePipelineLayout(
    const rhi::PipelineLayoutDesc& desc)
{
    rhi::PipelineLayoutHandle handle;

    if (desc.layouts.empty()) {
        return handle;
    }
    for (const auto& layout : desc.layouts) {
        if (layout.id >= m_BindGroupLayouts.size()) {
            return rhi::PipelineLayoutHandle{};
        }
    }

    std::vector<std::vector<D3D12_DESCRIPTOR_RANGE1>> resourceRanges(desc.layouts.size());
    std::vector<std::vector<D3D12_DESCRIPTOR_RANGE1>> samplerRanges(desc.layouts.size());
    std::vector<D3D12_ROOT_PARAMETER1> params;
    params.reserve(desc.layouts.size() * 2);

    for (std::size_t i = 0; i < desc.layouts.size(); ++i) {
        const auto& layoutDesc = m_BindGroupLayouts[desc.layouts[i].id].desc;
        const auto visibility = toD3D12(layoutDesc);

        for (const auto& binding : layoutDesc.shaderBindings) {
            D3D12_DESCRIPTOR_RANGE1 range{};
            range.RangeType = toD3D12(binding.type);
            range.NumDescriptors = binding.arraySize == 0 ? 1 : binding.arraySize;
            range.BaseShaderRegister = binding.binding;
            range.RegisterSpace = binding.set;
            range.Flags = D3D12_DESCRIPTOR_RANGE_FLAG_NONE;
            range.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

            if (binding.type == rhi::BindingType::Sampler) {
                samplerRanges[i].push_back(range);
            } else {
                resourceRanges[i].push_back(range);
            }
        }

        if (!resourceRanges[i].empty()) {
            D3D12_ROOT_PARAMETER1 param{};
            param.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
            param.ShaderVisibility = visibility;
            param.DescriptorTable.NumDescriptorRanges =
                static_cast<UINT>(resourceRanges[i].size());
            param.DescriptorTable.pDescriptorRanges = resourceRanges[i].data();
            params.push_back(param);
        }
        if (!samplerRanges[i].empty()) {
            D3D12_ROOT_PARAMETER1 param{};
            param.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
            param.ShaderVisibility = visibility;
            param.DescriptorTable.NumDescriptorRanges =
                static_cast<UINT>(samplerRanges[i].size());
            param.DescriptorTable.pDescriptorRanges = samplerRanges[i].data();
            params.push_back(param);
        }
    }

    if (params.empty()) {
        return handle;
    }

    D3D12_VERSIONED_ROOT_SIGNATURE_DESC versionedDesc{};
    versionedDesc.Version = D3D_ROOT_SIGNATURE_VERSION_1_1;
    versionedDesc.Desc_1_1.NumParameters = static_cast<UINT>(params.size());
    versionedDesc.Desc_1_1.pParameters = params.data();
    versionedDesc.Desc_1_1.Flags =
        D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

    Microsoft::WRL::ComPtr<ID3DBlob> signature;
    Microsoft::WRL::ComPtr<ID3DBlob> error;
    if (FAILED(D3D12SerializeVersionedRootSignature(
            &versionedDesc, &signature, &error))) {
        return rhi::PipelineLayoutHandle{};
    }

    Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature;
    if (FAILED(m_Device->CreateRootSignature(
            0, signature->GetBufferPointer(), signature->GetBufferSize(),
            IID_PPV_ARGS(&rootSignature)))) {
        return rhi::PipelineLayoutHandle{};
    }

    handle.id = static_cast<std::uint64_t>(m_PipelineLayouts.size());
    handle.desc = desc;
    m_PipelineLayouts.push_back(rootSignature);
    return handle;
}

rhi::GraphicsPipelineHandle DX12RHI::CreateGraphicsPipeline(
    const rhi::GraphicsPipelineDesc& desc)
{
    rhi::GraphicsPipelineHandle handle;

    if (desc.colorTargets.empty() || desc.colorTargets.size() > 8) {
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
    for (const auto& buffer : desc.vertexBuffers) {
        for (const auto& attr : buffer.attributes) {
            if (attr.bufferSlot >= desc.vertexBuffers.size()) {
                return handle;
            }
        }
    }

    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc{};
    psoDesc.pRootSignature = m_PipelineLayouts[desc.layout.id].Get();
    psoDesc.VS.pShaderBytecode = vsModule.bytecode.data();
    psoDesc.VS.BytecodeLength = vsModule.bytecode.size();
    psoDesc.PS.pShaderBytecode = fsModule.bytecode.data();
    psoDesc.PS.BytecodeLength = fsModule.bytecode.size();
    psoDesc.SampleMask = UINT_MAX;

    std::vector<D3D12_INPUT_ELEMENT_DESC> inputElements;
    for (const auto& buffer : desc.vertexBuffers) {
        for (const auto& attr : buffer.attributes) {
            D3D12_INPUT_ELEMENT_DESC element{};
            if (attr.location == 0) {
                element.SemanticName = "POSITION";
                element.SemanticIndex = 0;
            } else {
                element.SemanticName = "TEXCOORD";
                element.SemanticIndex = attr.location - 1;
            }
            element.Format = toD3D12(attr.format);
            if (element.Format == DXGI_FORMAT_UNKNOWN) {
                return handle;
            }
            element.InputSlot = attr.bufferSlot;
            element.AlignedByteOffset = attr.offset;
            element.InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
            element.InstanceDataStepRate = 0;
            inputElements.push_back(element);
        }
    }
    psoDesc.InputLayout.pInputElementDescs = inputElements.data();
    psoDesc.InputLayout.NumElements = static_cast<UINT>(inputElements.size());

    psoDesc.PrimitiveTopologyType = toD3D12(desc.topology);
    if (psoDesc.PrimitiveTopologyType == D3D12_PRIMITIVE_TOPOLOGY_TYPE_UNDEFINED) {
        return handle;
    }

    psoDesc.RasterizerState.FillMode = toD3D12(desc.fillMode);
    psoDesc.RasterizerState.CullMode = toD3D12(desc.cullMode);
    psoDesc.RasterizerState.FrontCounterClockwise =
        (desc.frontFace == rhi::FrontFace::CounterClockwise) ? TRUE : FALSE;
    psoDesc.RasterizerState.DepthClipEnable = TRUE;

    psoDesc.BlendState.AlphaToCoverageEnable = FALSE;
    psoDesc.BlendState.IndependentBlendEnable =
        (desc.colorTargets.size() > 1) ? TRUE : FALSE;
    for (std::size_t i = 0; i < desc.colorTargets.size(); ++i) {
        const auto& target = desc.colorTargets[i];
        auto& rt = psoDesc.BlendState.RenderTarget[i];
        rt.BlendEnable = target.blend.enabled ? TRUE : FALSE;
        rt.LogicOpEnable = FALSE;
        rt.SrcBlend = toD3D12(target.blend.srcColor);
        rt.DestBlend = toD3D12(target.blend.dstColor);
        rt.BlendOp = toD3D12(target.blend.colorOp);
        rt.SrcBlendAlpha = toD3D12(target.blend.srcAlpha);
        rt.DestBlendAlpha = toD3D12(target.blend.dstAlpha);
        rt.BlendOpAlpha = toD3D12(target.blend.alphaOp);
        rt.RenderTargetWriteMask = toD3D12(target.blend.writeMask);
    }

    if (desc.depthStencil.format == rhi::TextureFormat::Unknown) {
        psoDesc.DepthStencilState.DepthEnable = FALSE;
        psoDesc.DSVFormat = DXGI_FORMAT_UNKNOWN;
    } else {
        const DXGI_FORMAT dsvFormat = toD3D12(desc.depthStencil.format);
        if (dsvFormat == DXGI_FORMAT_UNKNOWN) {
            return handle;
        }
        psoDesc.DepthStencilState.DepthEnable =
            desc.depthStencil.depthTestEnabled ? TRUE : FALSE;
        psoDesc.DepthStencilState.DepthWriteMask =
            desc.depthStencil.depthWriteEnabled
                ? D3D12_DEPTH_WRITE_MASK_ALL
                : D3D12_DEPTH_WRITE_MASK_ZERO;
        psoDesc.DepthStencilState.DepthFunc = toD3D12(desc.depthStencil.depthFunc);
        psoDesc.DepthStencilState.StencilEnable = FALSE;
        psoDesc.DSVFormat = dsvFormat;
    }

    for (std::size_t i = 0; i < desc.colorTargets.size(); ++i) {
        const DXGI_FORMAT format = toD3D12(desc.colorTargets[i].format);
        if (format == DXGI_FORMAT_UNKNOWN) {
            return handle;
        }
        psoDesc.RTVFormats[i] = format;
    }
    psoDesc.NumRenderTargets = static_cast<UINT>(desc.colorTargets.size());
    psoDesc.SampleDesc.Count = toD3D12(desc.sampleCount);
    psoDesc.SampleDesc.Quality = 0;
    psoDesc.NodeMask = 0;
    psoDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;

    Microsoft::WRL::ComPtr<ID3D12PipelineState> pipeline;
    if (FAILED(m_Device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pipeline)))) {
        return handle;
    }

    handle.id = static_cast<std::uint64_t>(m_Pipelines.size());
    handle.desc = desc;
    m_Pipelines.push_back(pipeline);
    return handle;
}
