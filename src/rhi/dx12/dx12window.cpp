#include "rhi/dx12/dx12rhi.h"

#include <third_party/glfw/include/GLFW/glfw3.h>

rhi::WindowHandle DX12RHI::CreateNativeWindow(const rhi::WindowDesc& desc)
{
    if (!m_GlfwInitialized) {
        if (!glfwInit()) {
            return {};
        }
        m_GlfwInitialized = true;
    }
    if (desc.width == 0 || desc.height == 0) {
        return {};
    }

    glfwDefaultWindowHints();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, desc.resizable ? GLFW_TRUE : GLFW_FALSE);
    glfwWindowHint(GLFW_DECORATED, desc.decorated ? GLFW_TRUE : GLFW_FALSE);
    glfwWindowHint(GLFW_VISIBLE, desc.visible ? GLFW_TRUE : GLFW_FALSE);

    GLFWmonitor* monitor = nullptr;
    int width = static_cast<int>(desc.width);
    int height = static_cast<int>(desc.height);
    bool forceUndecorated = false;

    if (desc.mode != rhi::WindowMode::Windowed) {
        int monitorCount = 0;
        GLFWmonitor** monitors = glfwGetMonitors(&monitorCount);
        GLFWmonitor* picked = nullptr;
        if (monitorCount > 0) {
            int index = desc.monitorIndex;
            if (index < 0 || index >= monitorCount) {
                index = 0;
            }
            picked = monitors[index];
        } else {
            picked = glfwGetPrimaryMonitor();
        }

        if (picked != nullptr) {
            if (desc.mode == rhi::WindowMode::Fullscreen) {
                monitor = picked;
                if (const GLFWvidmode* videoMode = glfwGetVideoMode(picked)) {
                    width = videoMode->width;
                    height = videoMode->height;
                }
            } else {
                if (const GLFWvidmode* videoMode = glfwGetVideoMode(picked)) {
                    width = videoMode->width;
                    height = videoMode->height;
                }
                forceUndecorated = true;
            }
        }
    }

    if (forceUndecorated) {
        glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
    }

    GLFWwindow* window = glfwCreateWindow(width, height, desc.title.c_str(), monitor, nullptr);
    if (window == nullptr) {
        return {};
    }

    rhi::WindowHandle handle;
    handle.desc = desc;
    handle.desc.width = static_cast<std::uint32_t>(width);
    handle.desc.height = static_cast<std::uint32_t>(height);
    if (forceUndecorated) {
        handle.desc.decorated = false;
    }
    if (!m_FreeWindows.empty()) {
        handle.id = m_FreeWindows.back();
        m_FreeWindows.pop_back();
        m_Windows[handle.id] = window;
    } else {
        handle.id = static_cast<std::uint64_t>(m_Windows.size());
        m_Windows.push_back(window);
    }
    return handle;
}

void DX12RHI::PollEvents()
{
    if (m_GlfwInitialized) {
        glfwPollEvents();
    }
}

bool DX12RHI::WindowShouldClose(rhi::WindowHandle handle) const
{
    if (handle.id == 0xFFFFFFFFFFFFFFFF || handle.id >= m_Windows.size() ||
        m_Windows[handle.id] == nullptr) {
        return true;
    }
    return glfwWindowShouldClose(m_Windows[handle.id]) != 0;
}

void DX12RHI::DestroyNativeWindow(rhi::WindowHandle handle)
{
    if (handle.id == 0xFFFFFFFFFFFFFFFF || handle.id >= m_Windows.size() ||
        m_Windows[handle.id] == nullptr) {
        return;
    }
    glfwDestroyWindow(m_Windows[handle.id]);
    m_Windows[handle.id] = nullptr;
    m_FreeWindows.push_back(handle.id);
}
