cmake_minimum_required(VERSION 3.20)
project(RHI LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(RHI_BACKEND "VULKAN" CACHE STRING "RHI backend to use")

set(SOURCE_FILES
    src/main.cpp
    src/rhi/rhi.h
)

if(RHI_BACKEND STREQUAL "VULKAN")
  message(STATUS "Configuring RHI for: VULKAN")
  find_package(Vulkan REQUIRED)
  list(APPEND SOURCE_FILES src/rhi/vulkan/VkRHI.cpp)

elseif(RHI_BACKEND STREQUAL "DX12")
  if(NOT WIN32)
    message(FATAL_ERROR "DirectX 12 is only supported on Windows!")
  endif()
  message(STATUS "Configuring RHI for: DX12")
  list(APPEND SOURCE_FILES src/rhi/dx12/DX12RHI.cpp)

elseif(RHI_BACKEND STREQUAL "WEBGPU")
  message(STATUS "Configuring RHI for: WEBGPU")
  list(APPEND SOURCE_FILES src/rhi/webgpu/WGPURHI.cpp)
endif()

add_executable(${PROJECT_NAME} ${SOURCE_FILES})
target_include_directories(${PROJECT_NAME} PRIVATE src/)

if(RHI_BACKEND STREQUAL "VULKAN")
  target_link_libraries(${PROJECT_NAME} PRIVATE Vulkan::Vulkan)
  target_compile_definitions(${PROJECT_NAME} PRIVATE RHI_VULKAN)
elseif(RHI_BACKEND STREQUAL "DX12")
  target_link_libraries(${PROJECT_NAME} PRIVATE d3d12 dxgi)
  target_compile_definitions(${PROJECT_NAME} PRIVATE RHI_DX12)
endif()
