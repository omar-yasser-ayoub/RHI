cmake_minimum_required(VERSION 3.20)
project(RHI LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(RHI_BACKEND "VULKAN" CACHE STRING "RHI backend to use")
set_property(CACHE RHI_BACKEND PROPERTY STRINGS VULKAN DX12 WEBGPU)

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

# Include tests
add_subdirectory(tests)
