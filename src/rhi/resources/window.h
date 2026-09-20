#pragma once

#include <cstdint>
#include <string>

namespace rhi
{
    enum class WindowMode
    {
        Windowed,
        Borderless,
        Fullscreen,
    };

    struct WindowDesc
    {
        std::string title = "RHI";
        std::uint32_t width = 1280;
        std::uint32_t height = 720;

        WindowMode mode = WindowMode::Windowed;
        int monitorIndex = 0;

        bool resizable = true;
        bool decorated = true;
        bool visible = true;
    };

    struct WindowHandle
    {
        std::uint64_t id = 0xFFFFFFFFFFFFFFFF;
        WindowDesc desc{};
    };
}
