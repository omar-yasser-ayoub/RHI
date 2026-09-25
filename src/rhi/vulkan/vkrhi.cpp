#define VMA_IMPLEMENTATION
#include "rhi/vulkan/vkrhi.h"
#include <vulkan/vulkan.h>
#include <third_party/glfw/include/GLFW/glfw3.h>
#include <iostream>
#include <cstdint>
#include <cstring>
#include <vector>


bool VulkanRHI::Initialize() {
    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "reeka's RHI";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "N/A";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_3;

    VkInstanceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    std::uint32_t surfaceExtensionCount = 0;
    const char** surfaceExtensions =
        glfwGetRequiredInstanceExtensions(&surfaceExtensionCount);
    const char* fallbackExtensions[] = {
        VK_KHR_SURFACE_EXTENSION_NAME,
        "VK_KHR_win32_surface",
    };
    if (surfaceExtensions == nullptr || surfaceExtensionCount == 0) {
        surfaceExtensions = fallbackExtensions;
        surfaceExtensionCount = 2;
    }
    createInfo.enabledExtensionCount = surfaceExtensionCount;
    createInfo.ppEnabledExtensionNames = surfaceExtensions;

    VkResult result = vkCreateInstance(&createInfo, nullptr, &m_Instance);
    if (result != VK_SUCCESS) {
        std::cerr << "Failed to create Vulkan instance: " << result << std::endl;
        return false;
    }

    uint32_t deviceCount = 0;

    result = vkEnumeratePhysicalDevices(m_Instance, &deviceCount, nullptr);
    if (result != VK_SUCCESS) {
        std::cerr << "Failed to enumerate Vulkan devices: " << result << std::endl;
        return false;
    }
    if (deviceCount == 0) {
        std::cerr << "No Vulkan devices found." << std::endl;
        return false;
    }

    std::vector<VkPhysicalDevice> devices(deviceCount);
    result = vkEnumeratePhysicalDevices(m_Instance, &deviceCount, devices.data());

    if (result != VK_SUCCESS) {
        std::cerr << "Failed to enumerate Vulkan devices: " << result << std::endl;
        vkDestroyInstance(m_Instance, nullptr);
        m_Instance = VK_NULL_HANDLE;
        return false;
    }

    m_PhysicalDevice = devices[0];

    VkPhysicalDeviceProperties deviceProperties;
    vkGetPhysicalDeviceProperties(m_PhysicalDevice, &deviceProperties);

    m_Capabilities.deviceName = deviceProperties.deviceName;

    VkPhysicalDeviceMeshShaderFeaturesEXT meshFeatures{
        VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_FEATURES_EXT
    };

    VkPhysicalDeviceRayTracingPipelineFeaturesKHR rtFeatures{
        VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR
    };

    VkPhysicalDeviceFeatures2 features2{
        VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2
    };

    features2.pNext = &meshFeatures;
    meshFeatures.pNext = &rtFeatures;

    vkGetPhysicalDeviceFeatures2(m_PhysicalDevice, &features2);

    m_Capabilities.SupportsMeshShaders = meshFeatures.meshShader;
    m_Capabilities.SupportsRayTracing = rtFeatures.rayTracingPipeline;

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(
        m_PhysicalDevice,
        &queueFamilyCount,
        nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);

    vkGetPhysicalDeviceQueueFamilyProperties(
        m_PhysicalDevice,
        &queueFamilyCount,
        queueFamilies.data());

    uint32_t graphicsQueueFamily = UINT32_MAX;

    for (uint32_t i = 0; i < queueFamilyCount; ++i)
    {
        if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            graphicsQueueFamily = i;
            break;
        }
    }

    if (graphicsQueueFamily == UINT32_MAX) {
        std::cerr << "No graphics queue family found.\n";
        vkDestroyInstance(m_Instance, nullptr);
        m_Instance = VK_NULL_HANDLE;
        m_PhysicalDevice = VK_NULL_HANDLE;
        return false;
    }

    float priorities[] = { 1.0f, 1.0f };

    VkDeviceQueueCreateInfo queueInfo{};
    queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueInfo.queueFamilyIndex = graphicsQueueFamily;
    queueInfo.queueCount = 2;
    queueInfo.pQueuePriorities = priorities;

    const char* deviceExtensions[] = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
    };

    VkDeviceCreateInfo deviceInfo{};
    deviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceInfo.queueCreateInfoCount = 1;
    deviceInfo.pQueueCreateInfos = &queueInfo;
    deviceInfo.enabledExtensionCount = 1;
    deviceInfo.ppEnabledExtensionNames = deviceExtensions;

    VkPhysicalDeviceDynamicRenderingFeatures dynamicRendering{};
    dynamicRendering.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES;
    dynamicRendering.dynamicRendering = VK_TRUE;
    deviceInfo.pNext = &dynamicRendering;

    result = vkCreateDevice(
        m_PhysicalDevice,
        &deviceInfo,
        nullptr,
        &m_Device);

    if (result != VK_SUCCESS) {
        std::cerr << "Failed to create Vulkan device: " << result << '\n';
        vkDestroyInstance(m_Instance, nullptr);
        m_Instance = VK_NULL_HANDLE;
        m_PhysicalDevice = VK_NULL_HANDLE;
        return false;
    }

    vkGetDeviceQueue(m_Device, graphicsQueueFamily, 0, &m_GraphicsQueue);

    VmaAllocatorCreateInfo allocatorInfo{};
    allocatorInfo.physicalDevice = m_PhysicalDevice;
    allocatorInfo.device = m_Device;
    allocatorInfo.instance = m_Instance;
    allocatorInfo.vulkanApiVersion = VK_API_VERSION_1_3;
    if (vmaCreateAllocator(&allocatorInfo, &m_Allocator) != VK_SUCCESS) {
        std::cerr << "Failed to create VMA allocator\n";
        vkDestroyDevice(m_Device, nullptr);
        m_Device = VK_NULL_HANDLE;
        vkDestroyInstance(m_Instance, nullptr);
        m_Instance = VK_NULL_HANDLE;
        m_PhysicalDevice = VK_NULL_HANDLE;
        return false;
    }

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    poolInfo.maxSets = 100;

    VkDescriptorPoolSize poolSizes[] = {
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 100 },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 100 },
        { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 100 },
        { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 100 },
        { VK_DESCRIPTOR_TYPE_SAMPLER, 100 },
    };

    poolInfo.poolSizeCount = 5;
    poolInfo.pPoolSizes = poolSizes;

    vkCreateDescriptorPool(
        m_Device,
        &poolInfo,
        nullptr,
        &m_DescriptorPool
    );

    return true;
}

void VulkanRHI::Shutdown() {
    if (m_Device) {
        vkDeviceWaitIdle(m_Device);

        for (auto& data : m_Swapchains) {
            if (data.swapchain == VK_NULL_HANDLE) {
                continue;
            }
            for (VkImageView view : data.views) {
                if (view != VK_NULL_HANDLE) {
                    vkDestroyImageView(m_Device, view, nullptr);
                }
            }
            if (data.acquireFence != VK_NULL_HANDLE) {
                vkDestroyFence(m_Device, data.acquireFence, nullptr);
            }
            vkDestroySwapchainKHR(m_Device, data.swapchain, nullptr);
        }
        m_Swapchains.clear();
        m_FreeSwapchains.clear();

        for (std::size_t i = 0; i < m_Buffers.size(); ++i) {
            if (m_Buffers[i] != VK_NULL_HANDLE) {
                vmaDestroyBuffer(m_Allocator, m_Buffers[i], m_BufferMemory[i]);
                m_Buffers[i] = VK_NULL_HANDLE;
                m_BufferMemory[i] = nullptr;
            }
        }
        for (std::size_t i = 0; i < m_Textures.size(); ++i) {
            if (m_Textures[i] != VK_NULL_HANDLE) {
                vmaDestroyImage(m_Allocator, m_Textures[i], m_TextureMemory[i]);
                m_Textures[i] = VK_NULL_HANDLE;
                m_TextureMemory[i] = nullptr;
            }
            if (i < m_TextureViews.size() && m_TextureViews[i] != VK_NULL_HANDLE) {
                vkDestroyImageView(m_Device, m_TextureViews[i], nullptr);
                m_TextureViews[i] = VK_NULL_HANDLE;
            }
        }
        for (VkSampler sampler : m_Samplers) {
            if (sampler != VK_NULL_HANDLE) {
                vkDestroySampler(m_Device, sampler, nullptr);
            }
        }
        for (VkPipeline pipeline : m_Pipelines) {
            if (pipeline != VK_NULL_HANDLE) {
                vkDestroyPipeline(m_Device, pipeline, nullptr);
            }
        }
        for (VkPipelineLayout layout : m_PipelineLayouts) {
            if (layout != VK_NULL_HANDLE) {
                vkDestroyPipelineLayout(m_Device, layout, nullptr);
            }
        }

        m_Buffers.clear();
        m_BufferMemory.clear();
        m_Textures.clear();
        m_TextureMemory.clear();
        m_TextureViews.clear();
        m_Samplers.clear();
        m_Pipelines.clear();
        m_PipelineLayouts.clear();
        vmaDestroyAllocator(m_Allocator);
        m_Allocator = nullptr;
        vkDestroyDevice(m_Device, nullptr);
        m_Device = VK_NULL_HANDLE;
        m_GraphicsQueue = VK_NULL_HANDLE;
    }

    if (m_Instance) {
        for (VkSurfaceKHR surface : m_Surfaces) {
            if (surface != VK_NULL_HANDLE) {
                vkDestroySurfaceKHR(m_Instance, surface, nullptr);
            }
        }
        m_Surfaces.clear();
        m_FreeSurfaces.clear();
        vkDestroyInstance(m_Instance, nullptr);
        m_Instance = VK_NULL_HANDLE;
    }
    m_PhysicalDevice = VK_NULL_HANDLE;

    for (struct GLFWwindow* window : m_Windows) {
        if (window != nullptr) {
            glfwDestroyWindow(window);
        }
    }
    m_Windows.clear();
    m_FreeWindows.clear();
    if (m_GlfwInitialized) {
        glfwTerminate();
        m_GlfwInitialized = false;
    }
}


