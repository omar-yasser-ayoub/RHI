#include "rhi/resources/shader.h"
#include "rhi/webgpu/wgpurhi.h"
#include "third_party/spirv-reflect/spirv_reflect.h"

#include <cstdint>
#include <fstream>
#include <iterator>
#include <string>
#include <vector>

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
