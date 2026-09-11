#include "rhi/resources/shader.h"
#include "rhi/webgpu/wgpurhi.h"
#include "third_party/spirv-reflect/spirv_reflect.h"

#include <cstdint>
#include <fstream>
#include <iterator>
#include <string>
#include <vector>

namespace
{
rhi::ShaderReflection ParseReflection(
    SpvReflectShaderModule* reflection,
    rhi::ShaderStageFlags stage)
{
    rhi::ShaderReflection result;

    uint32_t bindingCount = 0;
    spvReflectEnumerateDescriptorBindings(reflection, &bindingCount, nullptr);

    std::vector<SpvReflectDescriptorBinding*> bindings(bindingCount);
    spvReflectEnumerateDescriptorBindings(
        reflection,
        &bindingCount,
        bindings.data());

    for (const auto* binding : bindings) {
        rhi::ShaderBinding bindingResult{};
        bindingResult.set = binding->set;
        bindingResult.binding = binding->binding;
        bindingResult.arraySize = binding->count;
        bindingResult.stage = stage;

        switch (binding->descriptor_type) {
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

    if (stage != rhi::ShaderStageFlags::Vertex) {
        return result;
    }

    uint32_t inputCount = 0;
    spvReflectEnumerateInputVariables(reflection, &inputCount, nullptr);
    std::vector<SpvReflectInterfaceVariable*> inputs(inputCount);
    spvReflectEnumerateInputVariables(reflection, &inputCount, inputs.data());

    for (const auto* input : inputs) {
        if (input->decoration_flags & SPV_REFLECT_DECORATION_BUILT_IN) {
            continue;
        }

        rhi::VertexInputAttr attr{};
        attr.location = input->location;

        switch (input->format) {
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

    return result;
}


}

rhi::ShaderModuleHandle WebGPURHI::CompileShader(rhi::ShaderSource source)
{
    rhi::ShaderModule module;

    const std::filesystem::path inputName = source.path.filename();
    const std::string filename = inputName.string();
    const std::string suffix = ".hlsl";
    if (filename.size() <= suffix.size() ||
        filename.compare(filename.size() - suffix.size(), suffix.size(), suffix) != 0) {
        return {static_cast<uint64_t>(m_ShaderModules.size())};
    }

    const std::string stem = filename.substr(0, filename.size() - suffix.size());
    const std::filesystem::path wgslPath =
        std::filesystem::path("shaders") / (stem + ".wgsl");

    const std::filesystem::path spirvPath =
        std::filesystem::path("shader_tools") / (stem + ".spv");

    std::ifstream wgslFile(wgslPath, std::ios::binary);
    if (!wgslFile) {
        return {static_cast<uint64_t>(m_ShaderModules.size())};
    }

    module.bytecode.assign(
        std::istreambuf_iterator<char>(wgslFile),
        std::istreambuf_iterator<char>());

    std::ifstream spirvFile(spirvPath, std::ios::binary);
    if (!spirvFile) {
        module.bytecode.clear();
        return {static_cast<uint64_t>(m_ShaderModules.size())};
    }

    std::vector<std::uint8_t> spirv{
        std::istreambuf_iterator<char>(spirvFile),
        std::istreambuf_iterator<char>()};
    if (spirv.empty()) {
        module.bytecode.clear();
        return {static_cast<uint64_t>(m_ShaderModules.size())};
    }

    SpvReflectShaderModule reflection{};
    const SpvReflectResult result = spvReflectCreateShaderModule(
        spirv.size(),
        spirv.data(),
        &reflection);
    if (result != SPV_REFLECT_RESULT_SUCCESS) {
        module.bytecode.clear();
        return {static_cast<uint64_t>(m_ShaderModules.size())};
    }

    module.reflection = ParseReflection(&reflection, source.stage);
    spvReflectDestroyShaderModule(&reflection);
    module.entryPoint = source.entryPoint.empty() ? "main" : source.entryPoint;
    m_ShaderModules.push_back(module);

    rhi::ShaderModuleHandle handle;
    handle.id = static_cast<uint64_t>(m_ShaderModules.size() - 1);
    return handle;
}

const rhi::ShaderModule* WebGPURHI::GetShaderModule(rhi::ShaderModuleHandle handle) const
{
    if (handle.id >= m_ShaderModules.size()) {
        return nullptr;
    }
    return &m_ShaderModules[handle.id];
}
