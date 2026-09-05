cmake_minimum_required(VERSION 3.20)

# Fetch Google Test
include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)
FetchContent_MakeAvailable(googletest)

# Create test executable
add_executable(rhi_tests
    test_rhi.cpp
)

# Link Google Test
target_link_libraries(rhi_tests
    PRIVATE
        gtest
        gtest_main
)

# Include directories
target_include_directories(rhi_tests
    PRIVATE
        ${CMAKE_SOURCE_DIR}/src
)



# Apply same configuration as main executable
if(RHI_BACKEND STREQUAL "VULKAN")
    find_package(Vulkan REQUIRED)
    target_sources(rhi_tests
        PRIVATE
            ${CMAKE_SOURCE_DIR}/src/rhi/vulkan/vkrhi.cpp
            ${CMAKE_SOURCE_DIR}/src/rhi/vulkan/vkconversions.cpp
            ${CMAKE_SOURCE_DIR}/src/rhi/vulkan/vkshader.cpp
            ${CMAKE_SOURCE_DIR}/src/third_party/spirv-reflect/spirv_reflect.cpp
            ${CMAKE_SOURCE_DIR}/src/rhi/vulkan/vktexture.cpp
            ${CMAKE_SOURCE_DIR}/src/rhi/vulkan/vkbuffer.cpp
            ${CMAKE_SOURCE_DIR}/src/rhi/vulkan/vksampler.cpp
    )
    target_link_libraries(rhi_tests
        PRIVATE
            Vulkan::Vulkan
            dxc
    )
    target_compile_definitions(rhi_tests
        PRIVATE
            RHI_VULKAN
    )
elseif(RHI_BACKEND STREQUAL "DX12")
    if(NOT WIN32)
        message(FATAL_ERROR "DirectX 12 is only supported on Windows.")
    endif()
    target_sources(rhi_tests
        PRIVATE
            ${CMAKE_SOURCE_DIR}/src/rhi/dx12/dx12rhi.cpp
            ${CMAKE_SOURCE_DIR}/src/rhi/dx12/dx12sampler.cpp
            ${CMAKE_SOURCE_DIR}/src/rhi/dx12/dx12texture.cpp
            ${CMAKE_SOURCE_DIR}/src/rhi/dx12/dx12buffer.cpp
            ${CMAKE_SOURCE_DIR}/src/rhi/dx12/dx12conversions.cpp
            ${CMAKE_SOURCE_DIR}/src/rhi/dx12/dx12shader.cpp
    )
    target_link_libraries(rhi_tests
        PRIVATE
            d3d12
            dxgi
            dxc
    )
    target_compile_definitions(rhi_tests
        PRIVATE
            RHI_DX12
    )
elseif(RHI_BACKEND STREQUAL "WEBGPU")
    set(DAWN_SDK_ROOT "C:/Dawn/SDK" CACHE PATH "Path to Dawn SDK")
    find_package(Threads REQUIRED)
    find_package(Dawn REQUIRED PATHS ${DAWN_SDK_ROOT})
    target_sources(rhi_tests
        PRIVATE
            ${CMAKE_SOURCE_DIR}/src/rhi/webgpu/wgpurhi.cpp
            ${CMAKE_SOURCE_DIR}/src/rhi/webgpu/wgpubuffer.cpp
            ${CMAKE_SOURCE_DIR}/src/rhi/webgpu/wgpuconversions.cpp
            ${CMAKE_SOURCE_DIR}/src/rhi/webgpu/wgpusampler.cpp
            ${CMAKE_SOURCE_DIR}/src/rhi/webgpu/wgputexture.cpp
            ${CMAKE_SOURCE_DIR}/src/rhi/webgpu/wgpushader.cpp
            ${CMAKE_SOURCE_DIR}/src/third_party/spirv-reflect/spirv_reflect.cpp
    )
    target_link_libraries(rhi_tests
        PRIVATE
            dawn::webgpu_dawn
    )
    target_compile_definitions(rhi_tests
        PRIVATE
            RHI_WEBGPU
    )
else()
    message(FATAL_ERROR "Unknown RHI backend: ${RHI_BACKEND}")
endif()

if(WIN32)
    add_custom_command(TARGET rhi_tests POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${DXC_ROOT}/bin/${DXC_ARCH}/dxcompiler.dll"
            "$<TARGET_FILE_DIR:rhi_tests>"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${DXC_ROOT}/bin/${DXC_ARCH}/dxil.dll"
            "$<TARGET_FILE_DIR:rhi_tests>"
    )
endif()

configure_rhi_shaders(rhi_tests)

# Enable testing
enable_testing()
add_test(NAME RHITests COMMAND rhi_tests)
