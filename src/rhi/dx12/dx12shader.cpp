#include "rhi/dx12/dx12rhi.h"
#include "rhi/resources/shader.h"
#include "third_party/dxc/inc/dxcapi.h"
#include <d3dcommon.h>
#include <iterator>
#include <fstream>
#include <string>
#include <wrl/client.h>
#include <d3d12shader.h>

rhi::ShaderReflection ParseReflection(
    ID3D12ShaderReflection* reflection,
    rhi::ShaderStageFlags stage
)
{
    rhi::ShaderReflection shaderReflection;

    D3D12_SHADER_DESC desc{};
    reflection->GetDesc(&desc);

    for (uint32_t i = 0; i < desc.BoundResources; ++i) {
        D3D12_SHADER_INPUT_BIND_DESC binding{};
        reflection->GetResourceBindingDesc(i, &binding);

        rhi::ShaderBinding bindingResult;
        bindingResult.set = binding.Space;
        bindingResult.binding = binding.BindPoint;
        bindingResult.stage = stage;
        bindingResult.arraySize = binding.BindCount;

        switch(binding.Type) {
            case D3D_SIT_CBUFFER:
                bindingResult.type = rhi::BindingType::UniformBuffer;
                break;

            case D3D_SIT_STRUCTURED:
                bindingResult.type = rhi::BindingType::StorageBuffer;
                break;

            case D3D_SIT_TEXTURE:
                bindingResult.type = rhi::BindingType::SampledTexture;
                break;

            case D3D_SIT_UAV_RWTYPED:
                bindingResult.type = rhi::BindingType::StorageTexture;
                break;

            case D3D_SIT_SAMPLER:
                bindingResult.type = rhi::BindingType::Sampler;
                break;
            default:
                continue;
        }
            shaderReflection.bindings.push_back(bindingResult);
    }

    if (stage == rhi::ShaderStageFlags::Vertex) {
        for (uint32_t i = 0; i < desc.InputParameters; ++i) {
            D3D12_SIGNATURE_PARAMETER_DESC input{};
            reflection->GetInputParameterDesc(i, &input);

            rhi::VertexInputAttr attr{};
            attr.location = input.Register;

            switch (input.ComponentType)
            {
            case D3D_REGISTER_COMPONENT_FLOAT32:
                switch (input.Mask)
                {
                case 1:  attr.format = rhi::VertexFormat::Float32;   break;
                case 3:  attr.format = rhi::VertexFormat::Float32x2; break;
                case 7:  attr.format = rhi::VertexFormat::Float32x3; break;
                case 15: attr.format = rhi::VertexFormat::Float32x4; break;
                default: continue;
                }
                break;

            case D3D_REGISTER_COMPONENT_SINT32:
                switch (input.Mask)
                {
                case 1:  attr.format = rhi::VertexFormat::Int32;   break;
                case 3:  attr.format = rhi::VertexFormat::Int32x2; break;
                case 7:  attr.format = rhi::VertexFormat::Int32x3; break;
                case 15: attr.format = rhi::VertexFormat::Int32x4; break;
                default: continue;
                }
                break;

            case D3D_REGISTER_COMPONENT_UINT32:
                switch (input.Mask)
                {
                case 1:  attr.format = rhi::VertexFormat::UInt32;   break;
                case 3:  attr.format = rhi::VertexFormat::UInt32x2; break;
                case 7:  attr.format = rhi::VertexFormat::UInt32x3; break;
                case 15: attr.format = rhi::VertexFormat::UInt32x4; break;
                default: continue;
                }
                break;

            default:
                continue;
            }

            shaderReflection.vertexInputs.push_back(attr);
        }
    }
    return shaderReflection;
}

rhi::ShaderModuleHandle DX12RHI::CompileShader(rhi::ShaderSource source) {

    std::filesystem::path path = source.path;

    std::ifstream file(path, std::ios::binary);

    std::string shadersource{
        std::istreambuf_iterator<char>(file),
        std::istreambuf_iterator<char>()
    };

    DxcBuffer buffer{};
    buffer.Ptr = shadersource.data();
    buffer.Size = shadersource.size();
    buffer.Encoding = DXC_CP_UTF8;

    std::wstring entryPoint(source.entryPoint.begin(), source.entryPoint.end());

    std::vector<LPCWSTR> args = {
        L"-E",
        entryPoint.c_str(),
    };

    if (source.stage == rhi::ShaderStageFlags::Vertex) {
        args.push_back(L"-T");
        args.push_back(L"vs_6_6");
    }
    else if (source.stage == rhi::ShaderStageFlags::Fragment) {
        args.push_back(L"-T");
        args.push_back(L"ps_6_6");
    }
    else if (source.stage == rhi::ShaderStageFlags::Compute) {
        args.push_back(L"-T");
        args.push_back(L"cs_6_6");
    }

    Microsoft::WRL::ComPtr<IDxcUtils> utils;

    HRESULT hr = DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(utils.ReleaseAndGetAddressOf()));

    if (FAILED(hr)) {
        printf("Failed to create DXC utils: 0x%x\n", hr);
        return {static_cast<uint64_t>(m_ShaderModules.size())};
    }

    Microsoft::WRL::ComPtr<IDxcIncludeHandler> includeHandler;

    hr = utils->CreateDefaultIncludeHandler(includeHandler.ReleaseAndGetAddressOf());

    if (FAILED(hr)) {
        printf("Failed to create DXC include handler: 0x%x\n", hr);
        return {static_cast<uint64_t>(m_ShaderModules.size())};
    }

    Microsoft::WRL::ComPtr<IDxcCompiler3> compiler3;

    hr = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&compiler3));

    if (FAILED(hr)) {
        printf("Failed to create DXC compiler: 0x%x\n", hr);
        return {static_cast<uint64_t>(m_ShaderModules.size())};
    }

    Microsoft::WRL::ComPtr<IDxcResult> result;

    hr = compiler3->Compile(
        &buffer,
        args.data(),
        static_cast<UINT32>(args.size()),
        includeHandler.Get(),
        IID_PPV_ARGS(&result)
    );

    if (FAILED(hr) || !result) {
        printf("Failed to compile shader: 0x%x\n", hr);
        return {static_cast<uint64_t>(m_ShaderModules.size())};
    }

    result->GetStatus(&hr);

    if (FAILED(hr)) {
        return {static_cast<uint64_t>(m_ShaderModules.size())};
    }

    Microsoft::WRL::ComPtr<IDxcBlobUtf8> errorMsgs;

    result->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&errorMsgs), nullptr);

    if (errorMsgs && errorMsgs->GetStringLength()) {
        printf("Shader compilation errors:\n%s\n", errorMsgs->GetStringPointer());
    }

    Microsoft::WRL::ComPtr<IDxcBlob> shaderobj;

    hr = result->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&shaderobj), nullptr);

    Microsoft::WRL::ComPtr<IDxcBlob> reflection;

    hr = result->GetOutput(DXC_OUT_REFLECTION, IID_PPV_ARGS(&reflection), nullptr);

    // now, we have to convert from dxc blob to rhi shader module
    rhi::ShaderModule module;
    module.bytecode.resize(shaderobj->GetBufferSize());
    memcpy(module.bytecode.data(), shaderobj->GetBufferPointer(), shaderobj->GetBufferSize());

    if (reflection) {
        DxcBuffer reflectionData{};
        reflectionData.Ptr = reflection->GetBufferPointer();
        reflectionData.Size = reflection->GetBufferSize();
        reflectionData.Encoding = 0;

        Microsoft::WRL::ComPtr<ID3D12ShaderReflection> shaderReflection;

        utils->CreateReflection(
            &reflectionData,
            IID_PPV_ARGS(&shaderReflection)
        );

        if (shaderReflection) {
            module.reflection = ParseReflection(shaderReflection.Get(), source.stage);
        }
    }

    m_ShaderModules.push_back(module);

    rhi::ShaderModuleHandle handle;
    handle.id = static_cast<uint64_t>(m_ShaderModules.size() - 1);
    return handle;
}

const rhi::ShaderModule* DX12RHI::GetShaderModule(rhi::ShaderModuleHandle handle) const
{
    if (handle.id >= m_ShaderModules.size()) {
        return nullptr;
    }
    return &m_ShaderModules[handle.id];
}
