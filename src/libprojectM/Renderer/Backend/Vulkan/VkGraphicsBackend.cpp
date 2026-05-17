#include "VkGraphicsBackend.hpp"

#ifdef PROJECTM_ENABLE_VULKAN

#include <Logging.hpp>

#include <cstring>
#include <stdexcept>
#include <vector>

namespace libprojectM {
namespace Renderer {
namespace Backend {
namespace Vulkan {

namespace {

const std::vector<const char*> s_requiredInstanceExtensions =
{
};

const std::vector<const char*> s_instanceExtensionsHdr =
{
    VK_EXT_SWAPCHAIN_COLOR_SPACE_EXTENSION_NAME
};

const std::vector<const char*> s_deviceExtensions =
{
    VK_KHR_SWAPCHAIN_EXTENSION_NAME,
};

const std::vector<const char*> s_deviceExtensionsHdr =
{
    VK_EXT_HDR_METADATA_EXTENSION_NAME,
};

VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT,
    VkDebugUtilsMessageTypeFlagsEXT,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void*)
{
    Logging::Log(std::string("Vulkan: ") + pCallbackData->pMessage, Logging::LogLevel::Warning);
    return VK_FALSE;
}

} // namespace

static auto s_vulkanBackendInitialized = false;

VkGraphicsBackend::VkGraphicsBackend()
{
    if (s_vulkanBackendInitialized)
        throw std::runtime_error("Only one Vulkan backend instance allowed");
    s_vulkanBackendInitialized = true;
    InitVulkan();
}

VkGraphicsBackend::~VkGraphicsBackend()
{
    CleanupVulkan();
    s_vulkanBackendInitialized = false;
}

void VkGraphicsBackend::InitVulkan()
{
    CreateInstance();
    PickPhysicalDevice();
    CreateDevice();
    Logging::Log("Vulkan backend initialized", Logging::LogLevel::Information);
}

void VkGraphicsBackend::CleanupVulkan()
{
    if (m_device != VK_NULL_HANDLE)
    {
        vkDeviceWaitIdle(m_device);
        vkDestroyDevice(m_device, nullptr);
        m_device = VK_NULL_HANDLE;
    }
    if (m_instance != VK_NULL_HANDLE)
    {
        vkDestroyInstance(m_instance, nullptr);
        m_instance = VK_NULL_HANDLE;
    }
}

void VkGraphicsBackend::CreateInstance()
{
    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "projectM";
    appInfo.applicationVersion = VK_MAKE_VERSION(4, 1, 0);
    appInfo.pEngineName = "projectM";
    appInfo.engineVersion = VK_MAKE_VERSION(4, 1, 0);
    appInfo.apiVersion = VK_API_VERSION_1_3;

    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;
    createInfo.enabledExtensionCount = static_cast<uint32_t>(s_requiredInstanceExtensions.size());
    createInfo.ppEnabledExtensionNames = s_requiredInstanceExtensions.data();

#ifdef _DEBUG
    const std::vector<const char*> validationLayers = {"VK_LAYER_KHRONOS_validation"};
    createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
    createInfo.ppEnabledLayerNames = validationLayers.data();
#else
    createInfo.enabledLayerCount = 0;
#endif

    if (vkCreateInstance(&createInfo, nullptr, &m_instance) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create Vulkan instance");
    }
}

void VkGraphicsBackend::PickPhysicalDevice()
{
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(m_instance, &deviceCount, nullptr);
    if (deviceCount == 0)
        throw std::runtime_error("No Vulkan-capable GPU found");

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(m_instance, &deviceCount, devices.data());

    m_physicalDevice = devices[0];
    // TODO: better device selection (discrete GPU preference, required features check)
}

void VkGraphicsBackend::CreateDevice()
{
    VkPhysicalDeviceFeatures deviceFeatures{};
    float queuePriority = 1.0f;

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(m_physicalDevice, &queueFamilyCount, nullptr);
    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(m_physicalDevice, &queueFamilyCount, queueFamilies.data());

    m_graphicsQueueFamily = 0;
    for (uint32_t i = 0; i < queueFamilyCount; i++)
    {
        if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            m_graphicsQueueFamily = i;
            break;
        }
    }

    VkDeviceQueueCreateInfo queueCreateInfo{};
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.queueFamilyIndex = m_graphicsQueueFamily;
    queueCreateInfo.queueCount = 1;
    queueCreateInfo.pQueuePriorities = &queuePriority;

    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.pQueueCreateInfos = &queueCreateInfo;
    createInfo.queueCreateInfoCount = 1;
    createInfo.pEnabledFeatures = &deviceFeatures;
    createInfo.enabledExtensionCount = static_cast<uint32_t>(s_deviceExtensions.size());
    createInfo.ppEnabledExtensionNames = s_deviceExtensions.data();

    if (vkCreateDevice(m_physicalDevice, &createInfo, nullptr, &m_device) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create Vulkan device");
    }

    vkGetDeviceQueue(m_device, m_graphicsQueueFamily, 0, &m_graphicsQueue);
}

//=== Stub implementations (will be filled in as we build the backend) ===

void VkGraphicsBackend::SetViewport(int, int, int, int) {}
void VkGraphicsBackend::ClearColor(float, float, float, float) {}
void VkGraphicsBackend::Clear() {}

void VkGraphicsBackend::SetBlendMode(BlendFactor, BlendFactor, BlendFactor, BlendFactor, bool) {}
void VkGraphicsBackend::DrawArrays(PrimitiveType, int, int) {}
void VkGraphicsBackend::DrawElements(PrimitiveType, int, IndexType, const void*) {}

void VkGraphicsBackend::SetLineWidth(float) {}
void VkGraphicsBackend::SetLineSmoothing(bool) {}
void VkGraphicsBackend::SetScissor(int, int, int, int, bool) {}

void VkGraphicsBackend::SetConstantVertexAttrib4f(unsigned int, float, float, float, float) {}
void VkGraphicsBackend::EnableVertexAttribArray(unsigned int, bool) {}

void VkGraphicsBackend::BindDefaultFramebuffer() {}
void VkGraphicsBackend::CopyFramebufferToTexture(int, int, int, int, int, int, int, int) {}

auto VkGraphicsBackend::GetShaderLanguageVersion() const -> GlslVersion
{
    return {4, 6}; // Vulkan uses SPIR-V, not GLSL
}

auto VkGraphicsBackend::CreateTexture(std::string, int, int, bool) -> std::shared_ptr<Texture>
{
    throw std::runtime_error("Vulkan backend: CreateTexture not yet implemented");
}

auto VkGraphicsBackend::CreateTexture(std::string, int, int, int, TextureFormat) -> std::shared_ptr<Texture>
{
    throw std::runtime_error("Vulkan backend: CreateTexture not yet implemented");
}

auto VkGraphicsBackend::CreateFramebuffer(int) -> std::shared_ptr<Framebuffer>
{
    throw std::runtime_error("Vulkan backend: CreateFramebuffer not yet implemented");
}

auto VkGraphicsBackend::CreateShaderProgram(std::string_view, std::string_view) -> std::shared_ptr<ShaderProgram>
{
    throw std::runtime_error("Vulkan backend: CreateShaderProgram not yet implemented");
}

auto VkGraphicsBackend::CreateSampler(SamplerWrap, SamplerFilter) -> std::shared_ptr<Sampler>
{
    throw std::runtime_error("Vulkan backend: CreateSampler not yet implemented");
}

auto VkGraphicsBackend::CreateMesh() -> std::shared_ptr<VertexMesh>
{
    throw std::runtime_error("Vulkan backend: CreateMesh not yet implemented");
}

} // namespace Vulkan
} // namespace Backend
} // namespace Renderer
} // namespace libprojectM

#endif // PROJECTM_ENABLE_VULKAN
