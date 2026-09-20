cmake_minimum_required(VERSION 3.20)
project(RHI LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(RHI_BACKEND "VULKAN" CACHE STRING "RHI backend to use")
set_property(CACHE RHI_BACKEND PROPERTY STRINGS VULKAN DX12 WEBGPU)

set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
add_subdirectory(src/third_party/glfw)

set(RHI_SHADER_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src/shaders")
set(RHI_SHADER_BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}/shaders")
file(GLOB RHI_SHADER_SOURCES CONFIGURE_DEPENDS
    "${RHI_SHADER_SOURCE_DIR}/*.hlsl")

function(configure_rhi_shaders target)
  if(NOT RHI_SHADER_SOURCES)
    message(WARNING "No HLSL shaders found in ${RHI_SHADER_SOURCE_DIR}")
    return()
  endif()

  if(NOT TARGET rhi_shaders)
    set(shader_outputs)

    if(RHI_BACKEND STREQUAL "WEBGPU")
      if(NOT WIN32)
        message(FATAL_ERROR
          "WebGPU shader compilation requires Windows DXC and Tint tools")
      endif()

      set(shader_tool_dir "${CMAKE_CURRENT_BINARY_DIR}/shader_tools")
      set(dxc_exe "${DXC_ROOT}/bin/${DXC_ARCH}/dxc.exe")
      set(tint_exe "${CMAKE_CURRENT_SOURCE_DIR}/src/third_party/tint/tint.exe")

      foreach(shader IN LISTS RHI_SHADER_SOURCES)
        get_filename_component(shader_filename "${shader}" NAME)

        if(shader_filename MATCHES "^(.+)\\.vert\\.hlsl$")
          set(shader_stem "${CMAKE_MATCH_1}.vert")
          set(shader_profile vs_6_6)
        elseif(shader_filename MATCHES "^(.+)\\.frag\\.hlsl$")
          set(shader_stem "${CMAKE_MATCH_1}.frag")
          set(shader_profile ps_6_6)
        elseif(shader_filename MATCHES "^(.+)\\.comp\\.hlsl$")
          set(shader_stem "${CMAKE_MATCH_1}.comp")
          set(shader_profile cs_6_6)
        else()
          message(FATAL_ERROR
            "Shader '${shader_filename}' must use .vert.hlsl, .frag.hlsl, or .comp.hlsl")
        endif()

        set(hlsl_output "${RHI_SHADER_BINARY_DIR}/${shader_filename}")
        set(spirv_output "${shader_tool_dir}/${shader_stem}.spv")
        set(wgsl_output "${RHI_SHADER_BINARY_DIR}/${shader_stem}.wgsl")
        set(reflection_output "${RHI_SHADER_BINARY_DIR}/${shader_stem}.reflection.txt")

        add_custom_command(
          OUTPUT "${wgsl_output}" "${reflection_output}"
          BYPRODUCTS "${spirv_output}"
          COMMAND ${CMAKE_COMMAND} -E make_directory "${RHI_SHADER_BINARY_DIR}"
          COMMAND ${CMAKE_COMMAND} -E make_directory "${shader_tool_dir}"
          COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${shader}" "${hlsl_output}"
          COMMAND "${dxc_exe}"
            -spirv -E main -T ${shader_profile} -fvk-use-dx-layout
            -Fo "${spirv_output}" "${shader}"
          COMMAND "${tint_exe}"
            --format wgsl -o "${wgsl_output}" "${spirv_output}"
          COMMAND "${tint_exe}"
            --dump-inspector-bindings true "${spirv_output}"
            > "${reflection_output}"
          DEPENDS "${shader}" "${dxc_exe}" "${tint_exe}"
          VERBATIM
        )
        list(APPEND shader_outputs "${wgsl_output}" "${reflection_output}")
      endforeach()
    else()
      foreach(shader IN LISTS RHI_SHADER_SOURCES)
        get_filename_component(shader_filename "${shader}" NAME)
        set(hlsl_output "${RHI_SHADER_BINARY_DIR}/${shader_filename}")
        add_custom_command(
          OUTPUT "${hlsl_output}"
          COMMAND ${CMAKE_COMMAND} -E make_directory "${RHI_SHADER_BINARY_DIR}"
          COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${shader}" "${hlsl_output}"
          DEPENDS "${shader}"
          VERBATIM
        )
        list(APPEND shader_outputs "${hlsl_output}")
      endforeach()
    endif()

    add_custom_target(rhi_shaders DEPENDS ${shader_outputs})
    if(TARGET rhi_shader_cleanup)
      add_dependencies(rhi_shaders rhi_shader_cleanup)
    endif()
  endif()

  add_dependencies(${target} rhi_shaders)
  target_compile_definitions(${target}
    PRIVATE RHI_SHADER_DIR=\"${RHI_SHADER_BINARY_DIR}\")
endfunction()

set(SOURCE_FILES
    src/main.cpp
)

add_executable(${PROJECT_NAME} ${SOURCE_FILES})

target_include_directories(${PROJECT_NAME}
    PRIVATE
        src
)

if(WIN32)
  set(DXC_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/src/third_party/dxc" CACHE PATH "DXC SDK root")
  if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(DXC_ARCH x64)
  else()
    set(DXC_ARCH x86)
  endif()

  add_library(dxc SHARED IMPORTED GLOBAL)
  set_target_properties(dxc PROPERTIES
    IMPORTED_IMPLIB "${DXC_ROOT}/lib/${DXC_ARCH}/dxcompiler.lib"
    IMPORTED_LOCATION "${DXC_ROOT}/bin/${DXC_ARCH}/dxcompiler.dll"
    INTERFACE_INCLUDE_DIRECTORIES "${DXC_ROOT}/inc"
  )
endif()

if(RHI_BACKEND STREQUAL "VULKAN")
  message(STATUS "Configuring RHI for Vulkan")
  find_package(Vulkan REQUIRED)
  target_sources(${PROJECT_NAME}
        PRIVATE
            src/rhi/vulkan/vkrhi.cpp
            src/rhi/vulkan/vktexture.cpp
            src/rhi/vulkan/vkbuffer.cpp
            src/rhi/vulkan/vksampler.cpp
            src/rhi/vulkan/vkconversions.cpp
            src/rhi/vulkan/vkshader.cpp
            src/rhi/vulkan/vkbindgroup.cpp
            src/rhi/vulkan/vkpipeline.cpp
            src/rhi/vulkan/vksurface.cpp
            src/rhi/vulkan/vkwindow.cpp
            src/third_party/spirv-reflect/spirv_reflect.cpp
    )
  target_link_libraries(${PROJECT_NAME}
        PRIVATE
            Vulkan::Vulkan
            dxc
    )
  target_compile_definitions(${PROJECT_NAME}
        PRIVATE
            RHI_VULKAN
    )
elseif(RHI_BACKEND STREQUAL "DX12")
  if(NOT WIN32)
    message(FATAL_ERROR "DirectX 12 is only supported on Windows.")
  endif()
  message(STATUS "Configuring RHI for DirectX 12")
  target_sources(${PROJECT_NAME}
        PRIVATE
            src/rhi/dx12/dx12rhi.cpp
            src/rhi/dx12/dx12sampler.cpp
            src/rhi/dx12/dx12texture.cpp
            src/rhi/dx12/dx12buffer.cpp
            src/rhi/dx12/dx12conversions.cpp
            src/rhi/dx12/dx12shader.cpp
            src/rhi/dx12/dx12bindgroup.cpp
            src/rhi/dx12/dx12pipeline.cpp
            src/rhi/dx12/dx12surface.cpp
            src/rhi/dx12/dx12window.cpp
    )
  target_link_libraries(${PROJECT_NAME}
        PRIVATE
            d3d12
            dxgi
            dxc
    )
  target_compile_definitions(${PROJECT_NAME}
        PRIVATE
            RHI_DX12
    )
elseif(RHI_BACKEND STREQUAL "WEBGPU")
  set(DAWN_SDK_ROOT "C:/Dawn/SDK" CACHE PATH "Path to Dawn SDK")
  find_package(Threads REQUIRED)
  find_package(Dawn REQUIRED PATHS ${DAWN_SDK_ROOT})
  target_sources(${PROJECT_NAME}
            PRIVATE
                src/rhi/webgpu/wgpurhi.cpp
                src/rhi/webgpu/wgpubuffer.cpp
                src/rhi/webgpu/wgpuconversions.cpp
                src/rhi/webgpu/wgpusampler.cpp
                src/rhi/webgpu/wgputexture.cpp
                src/rhi/webgpu/wgpushader.cpp
                src/rhi/webgpu/wgpubindgroup.cpp
                src/rhi/webgpu/wgpupipeline.cpp
                src/rhi/webgpu/wgpusurface.cpp
                src/rhi/webgpu/wgpuwindow.cpp
                src/third_party/spirv-reflect/spirv_reflect.cpp
        )
  target_link_libraries(${PROJECT_NAME}
            PRIVATE
                dawn::webgpu_dawn
        )
  target_compile_definitions(${PROJECT_NAME}
            PRIVATE
                RHI_WEBGPU
        )
else()
  message(FATAL_ERROR "Unknown RHI backend: ${RHI_BACKEND}")
endif()

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        glfw
)

if(WIN32)
  add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
      "${DXC_ROOT}/bin/${DXC_ARCH}/dxcompiler.dll"
      "$<TARGET_FILE_DIR:${PROJECT_NAME}>"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
      "${DXC_ROOT}/bin/${DXC_ARCH}/dxil.dll"
      "$<TARGET_FILE_DIR:${PROJECT_NAME}>"
  )
endif()

configure_rhi_shaders(${PROJECT_NAME})

# Include tests
add_subdirectory(tests)
