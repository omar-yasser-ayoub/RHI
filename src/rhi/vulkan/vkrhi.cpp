#include "rhi/rhi.h"
#include <vulkan/vulkan.h>
#include <iostream>

RHI::RHI() {

}

RHI::~RHI() {
}

bool RHI::Initialize() {
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
        return false;
    }

    VkPhysicalDevice physicalDevice = devices[0];

    VkPhysicalDeviceProperties deviceProperties;
    vkGetPhysicalDeviceProperties(selectedDevice, &deviceProperties);

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

    vkGetPhysicalDeviceFeatures2(selectedDevice, &features2);

    m_Capabilities.SupportsMeshShaders = meshFeatures.meshShader;
    m_Capabilities.SupportsRayTracing = rtFeatures.rayTracingPipeline;

    return true;
}

void RHI::Shutdown() {
    if (m_Instance) {
        vkDestroyInstance(m_Instance, nullptr);
        m_Instance = VK_NULL_HANDLE;
    }
}

void RHI::PrintCapabilities()
{
    std::cout << "========== Vulkan RHI Capabilities ==========\n";
    std::cout << "Device Name        : " << m_Capabilities.deviceName << '\n';
    std::cout << "Mesh Shaders       : "
              << (m_Capabilities.SupportsMeshShaders ? "Supported" : "Not Supported") << '\n';
    std::cout << "Ray Tracing        : "
              << (m_Capabilities.SupportsRayTracing ? "Supported" : "Not Supported") << '\n';
    std::cout << "=============================================\n";
}

const RHICapabilities& RHI::GetCapabilities() const {
    return m_Capabilities;
}
