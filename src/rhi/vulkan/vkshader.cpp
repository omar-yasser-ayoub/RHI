#include "rhi/resources/shader.h"
#include "rhi/vulkan/vkrhi.h"
#include <wrl/client.h>
#include "third_party/spirv-reflect/spirv_reflect.h"

#include "third_party/dxc/inc/dxcapi.h"
#include <d3dcommon.h>
#include <iterator>
#include <fstream>
#include <string>
#include <wrl/client.h>

rhi::ShaderReflection ParseReflection(
    SpvReflectShaderModule* reflection,
    rhi::ShaderStageFlags stage
)
{
    rhi::ShaderReflection result;

    uint32_t bindingCount = 0;

    spvReflectEnumerateDescriptorBindings(reflection, &bindingCount, nullptr);

    std::vector<SpvReflectDescriptorBinding*> bindings(bindingCount);
    spvReflectEnumerateDescriptorBindings(reflection, &bindingCount, bindings.data());

    for (SpvReflectDescriptorBinding* binding : bindings) {
        rhi::ShaderBinding bindingResult{};

        bindingResult.set = binding->set;
        bindingResult.binding = binding->binding;
        bindingResult.arraySize = binding->count;
        bindingResult.stage = stage;

        switch (binding->descriptor_type)
        {
            case SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
                bindingResult.type = rhi::BindingType::UniformBuffer;
                break;

            case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_BUFFER:
                bindingResult.type = rhi::BindingType::StorageBuffer;
                break;

            case SPV_REFLECT_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
            case SPV_REFLECT_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
                bindingResult.type = rhi::BindingType::SampledTexture;
                break;

            case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_IMAGE:
                bindingResult.type = rhi::BindingType::StorageTexture;
                break;

            case SPV_REFLECT_DESCRIPTOR_TYPE_SAMPLER:
                bindingResult.type = rhi::BindingType::Sampler;
                break;

            default:
                continue;
        }

        result.bindings.push_back(bindingResult);
    }

    // Vertex inputs
        if (stage == rhi::ShaderStageFlags::Vertex)
        {
            uint32_t inputCount = 0;

            spvReflectEnumerateInputVariables(
                reflection,
                &inputCount,
                nullptr
            );

            std::vector<SpvReflectInterfaceVariable*> inputs(inputCount);

            spvReflectEnumerateInputVariables(
                reflection,
                &inputCount,
                inputs.data()
            );

            for (SpvReflectInterfaceVariable* input : inputs)
            {
                // Built-in inputs such as gl_VertexIndex aren't vertex attributes.
                if (input->decoration_flags & SPV_REFLECT_DECORATION_BUILT_IN)
                    continue;

                rhi::VertexInputAttr attr{};
                attr.location = input->location;

                switch (input->format)
                {
                case SPV_REFLECT_FORMAT_R32_SFLOAT:
                    attr.format = rhi::VertexFormat::Float32;
                    break;

                case SPV_REFLECT_FORMAT_R32G32_SFLOAT:
                    attr.format = rhi::VertexFormat::Float32x2;
                    break;

                case SPV_REFLECT_FORMAT_R32G32B32_SFLOAT:
                    attr.format = rhi::VertexFormat::Float32x3;
                    break;

                case SPV_REFLECT_FORMAT_R32G32B32A32_SFLOAT:
                    attr.format = rhi::VertexFormat::Float32x4;
                    break;

                case SPV_REFLECT_FORMAT_R32_SINT:
                    attr.format = rhi::VertexFormat::Int32;
                    break;

                case SPV_REFLECT_FORMAT_R32G32_SINT:
                    attr.format = rhi::VertexFormat::Int32x2;
                    break;

                case SPV_REFLECT_FORMAT_R32G32B32_SINT:
                    attr.format = rhi::VertexFormat::Int32x3;
                    break;

                case SPV_REFLECT_FORMAT_R32G32B32A32_SINT:
                    attr.format = rhi::VertexFormat::Int32x4;
                    break;

                case SPV_REFLECT_FORMAT_R32_UINT:
                    attr.format = rhi::VertexFormat::UInt32;
                    break;

                case SPV_REFLECT_FORMAT_R32G32_UINT:
                    attr.format = rhi::VertexFormat::UInt32x2;
                    break;

                case SPV_REFLECT_FORMAT_R32G32B32_UINT:
                    attr.format = rhi::VertexFormat::UInt32x3;
                    break;

                case SPV_REFLECT_FORMAT_R32G32B32A32_UINT:
                    attr.format = rhi::VertexFormat::UInt32x4;
                    break;

                default:
                    continue;
                }

                result.vertexInputs.push_back(attr);
            }
        }

        return result;
}

rhi::ShaderModuleHandle VulkanRHI::CompileShader(rhi::ShaderSource source) {

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
        L"-spirv",
        L"-fspv-target-env=vulkan1.3",
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

    // now, we have to convert from dxc blob to rhi shader module
    rhi::ShaderModule module;
    module.bytecode.resize(shaderobj->GetBufferSize());
    memcpy(module.bytecode.data(), shaderobj->GetBufferPointer(), shaderobj->GetBufferSize());

    SpvReflectShaderModule reflectModule{};

    SpvReflectResult reflectionResult = spvReflectCreateShaderModule(
        module.bytecode.size(),
        module.bytecode.data(),
        &reflectModule);

    rhi::ShaderReflection shaderReflection = ParseReflection(&reflectModule, source.stage);

    module.reflection = shaderReflection;

    spvReflectDestroyShaderModule(&reflectModule);

    m_ShaderModules.push_back(module);

    rhi::ShaderModuleHandle handle;
    handle.id = static_cast<uint64_t>(m_ShaderModules.size() - 1);
    return handle;
}
