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
        return {};
    }

    const std::string stem = filename.substr(0, filename.size() - suffix.size());
    const std::filesystem::path wgslPath =
        std::filesystem::path("shaders") / (stem + ".wgsl");

    const std::filesystem::path spirvPath =
        std::filesystem::path("shader_tools") / (stem + ".spv");

    std::ifstream wgslFile(wgslPath, std::ios::binary);
    if (!wgslFile) {
        return {};
    }

    module.bytecode.assign(
        std::istreambuf_iterator<char>(wgslFile),
        std::istreambuf_iterator<char>());

    std::ifstream spirvFile(spirvPath, std::ios::binary);
    if (!spirvFile) {
        module.bytecode.clear();
        return {};
    }

    std::vector<std::uint8_t> spirv{
        std::istreambuf_iterator<char>(spirvFile),
        std::istreambuf_iterator<char>()};
    if (spirv.empty()) {
        module.bytecode.clear();
        return {};
    }

    SpvReflectShaderModule reflection{};
    const SpvReflectResult result = spvReflectCreateShaderModule(
        spirv.size(),
        spirv.data(),
        &reflection);
    if (result != SPV_REFLECT_RESULT_SUCCESS) {
        module.bytecode.clear();
        return {};
    }

    module.reflection = ParseReflection(&reflection, source.stage);
    spvReflectDestroyShaderModule(&reflection);
    module.entryPoint = source.entryPoint.empty() ? "main" : source.entryPoint;

    rhi::ShaderModuleHandle handle;
    if (!m_FreeShaderModules.empty()) {
        handle.id = m_FreeShaderModules.back();
        m_FreeShaderModules.pop_back();
        m_ShaderModules[handle.id] = module;
    } else {
        handle.id = static_cast<uint64_t>(m_ShaderModules.size());
        m_ShaderModules.push_back(module);
    }
    return handle;
}

void WebGPURHI::DestroyShaderModule(rhi::ShaderModuleHandle handle)
{
    if (handle.id >= m_ShaderModules.size() ||
        m_ShaderModules[handle.id].bytecode.empty()) {
        return;
    }
    m_ShaderModules[handle.id] = {};
    m_FreeShaderModules.push_back(handle.id);
}

const rhi::ShaderModule* WebGPURHI::GetShaderModule(rhi::ShaderModuleHandle handle) const
{
    if (handle.id >= m_ShaderModules.size() ||
        m_ShaderModules[handle.id].bytecode.empty()) {
        return nullptr;
    }
    return &m_ShaderModules[handle.id];
}
