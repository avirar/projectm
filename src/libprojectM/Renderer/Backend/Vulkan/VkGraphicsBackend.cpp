#include "VkGraphicsBackend.hpp"

#ifdef PROJECTM_ENABLE_VULKAN

#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>

#include <Logging.hpp>

#include <glm/glm.hpp>

#include <array>
#include <cstring>
#include <stdexcept>
#include <vector>

namespace libprojectM {
namespace Renderer {
namespace Backend {
namespace Vulkan {

// using namespace Logging;  -- Logging is a class, not a namespace

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

auto ToVkPrimitiveTopology(PrimitiveType type) -> VkPrimitiveTopology
{
    switch (type)
    {
        case PrimitiveType::Points: return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
        case PrimitiveType::Lines: return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
        case PrimitiveType::LineStrip: return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
        case PrimitiveType::LineLoop: return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
        case PrimitiveType::Triangles: return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        case PrimitiveType::TriangleStrip: return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
        case PrimitiveType::TriangleFan: return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN;
    }
    return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
}

auto ToVkBlendFactor(BlendFactor factor) -> VkBlendFactor
{
    switch (factor)
    {
        case BlendFactor::Zero: return VK_BLEND_FACTOR_ZERO;
        case BlendFactor::One: return VK_BLEND_FACTOR_ONE;
        case BlendFactor::SrcColor: return VK_BLEND_FACTOR_SRC_COLOR;
        case BlendFactor::OneMinusSrcColor: return VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;
        case BlendFactor::DstColor: return VK_BLEND_FACTOR_DST_COLOR;
        case BlendFactor::OneMinusDstColor: return VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR;
        case BlendFactor::SrcAlpha: return VK_BLEND_FACTOR_SRC_ALPHA;
        case BlendFactor::OneMinusSrcAlpha: return VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        case BlendFactor::DstAlpha: return VK_BLEND_FACTOR_DST_ALPHA;
        case BlendFactor::OneMinusDstAlpha: return VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
        case BlendFactor::ConstantColor: return VK_BLEND_FACTOR_CONSTANT_COLOR;
        case BlendFactor::OneMinusConstantColor: return VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR;
        case BlendFactor::ConstantAlpha: return VK_BLEND_FACTOR_CONSTANT_ALPHA;
        case BlendFactor::OneMinusConstantAlpha: return VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_ALPHA;
        case BlendFactor::SrcAlphaSaturate: return VK_BLEND_FACTOR_SRC_ALPHA_SATURATE;
        default: return VK_BLEND_FACTOR_ONE;
    }
}

auto ToVkSamplerAddressMode(SamplerWrap wrap) -> VkSamplerAddressMode
{
    switch (wrap)
    {
        case SamplerWrap::Repeat: return VK_SAMPLER_ADDRESS_MODE_REPEAT;
        case SamplerWrap::MirroredRepeat: return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
        case SamplerWrap::ClampToEdge: return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        case SamplerWrap::ClampToBorder: return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
    }
    return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
}

auto ToVkFilter(SamplerFilter filter) -> VkFilter
{
    switch (filter)
    {
        case SamplerFilter::Nearest: return VK_FILTER_NEAREST;
        case SamplerFilter::Linear: return VK_FILTER_LINEAR;
        default: return VK_FILTER_LINEAR;
    }
}

auto ToVkFormat(TextureFormat format) -> VkFormat
{
    switch (format)
    {
        case TextureFormat::R8: return VK_FORMAT_R8_UNORM;
        case TextureFormat::RG8: return VK_FORMAT_R8G8_UNORM;
        case TextureFormat::RGB8: return VK_FORMAT_R8G8B8_UNORM;
        case TextureFormat::RGBA8: return VK_FORMAT_R8G8B8A8_UNORM;
        case TextureFormat::R16F: return VK_FORMAT_R16_SFLOAT;
        case TextureFormat::RG16F: return VK_FORMAT_R16G16_SFLOAT;
        case TextureFormat::RGB16F: return VK_FORMAT_R16G16B16_SFLOAT;
        case TextureFormat::RGBA16F: return VK_FORMAT_R16G16B16A16_SFLOAT;
        case TextureFormat::R32F: return VK_FORMAT_R32_SFLOAT;
        case TextureFormat::RG32F: return VK_FORMAT_R32G32_SFLOAT;
        case TextureFormat::RGB32F: return VK_FORMAT_R32G32B32_SFLOAT;
        case TextureFormat::RGBA32F: return VK_FORMAT_R32G32B32A32_SFLOAT;
        case TextureFormat::R10G10B10A2: return VK_FORMAT_A2R10G10B10_UNORM_PACK32;
        case TextureFormat::SRGB8_ALPHA8: return VK_FORMAT_R8G8B8A8_SRGB;
    }
    return VK_FORMAT_R8G8B8A8_UNORM;
}

auto ToVkIndexType(IndexType type) -> VkIndexType
{
    switch (type)
    {
        case IndexType::UnsignedByte: return VK_INDEX_TYPE_UINT8_EXT;
        case IndexType::UnsignedShort: return VK_INDEX_TYPE_UINT16;
        case IndexType::UnsignedInt: return VK_INDEX_TYPE_UINT32;
    }
    return VK_INDEX_TYPE_UINT32;
}

} // namespace

static auto s_vulkanBackendInitialized = false;

//====================================================================
// VkGraphicsBackend - Lifecycle
//====================================================================

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
    InitAllocator();
    InitCommandPool();

    // Load extension function pointers
    m_vkSetHdrMetadataEXT = reinterpret_cast<PFN_vkSetHdrMetadataEXT>(
        vkGetInstanceProcAddr(m_instance, "vkSetHdrMetadataEXT"));

    Logging::Log("Vulkan backend initialized", Logging::LogLevel::Information);
}

void VkGraphicsBackend::CleanupVulkan()
{
    if (m_device != VK_NULL_HANDLE)
    {
        vkDeviceWaitIdle(m_device);

        if (m_frameFence) { vkDestroyFence(m_device, m_frameFence, nullptr); }
        if (m_commandPool) { vkDestroyCommandPool(m_device, m_commandPool, nullptr); }
        DestroySwapChain();

        if (m_allocator) { vmaDestroyAllocator(m_allocator); }
        vkDestroyDevice(m_device, nullptr);
        m_device = VK_NULL_HANDLE;
    }
    if (m_instance != VK_NULL_HANDLE)
    {
        vkDestroyInstance(m_instance, nullptr);
        m_instance = VK_NULL_HANDLE;
    }
}

//====================================================================
// Vulkan Setup
//====================================================================

void VkGraphicsBackend::CreateInstance()
{
    std::vector<const char*> extensions(s_requiredInstanceExtensions.begin(), s_requiredInstanceExtensions.end());

    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "projectM";
    appInfo.applicationVersion = VK_MAKE_VERSION(4, 1, 0);
    appInfo.pEngineName = "projectM";
    appInfo.engineVersion = VK_MAKE_VERSION(4, 1, 0);
    appInfo.apiVersion = VK_API_VERSION_1_3;

    // Check for HDR instance extensions
    uint32_t instExtCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &instExtCount, nullptr);
    std::vector<VkExtensionProperties> instExts(instExtCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &instExtCount, instExts.data());

    for (const auto& hdrExt : s_instanceExtensionsHdr)
    {
        for (const auto& avail : instExts)
        {
            if (strcmp(avail.extensionName, hdrExt) == 0)
            {
                extensions.push_back(hdrExt);
                Logging::Log(std::string("Vulkan: HDR instance extension enabled: ") + hdrExt, Logging::LogLevel::Information);
                break;
            }
        }
    }

    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;
    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();

#ifdef _DEBUG
    const std::vector<const char*> validationLayers = {"VK_LAYER_KHRONOS_validation"};
    createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
    createInfo.ppEnabledLayerNames = validationLayers.data();
#else
    createInfo.enabledLayerCount = 0;
#endif

    if (vkCreateInstance(&createInfo, nullptr, &m_instance) != VK_SUCCESS)
        throw std::runtime_error("Failed to create Vulkan instance");
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
    std::string deviceName = "Vulkan device selected (index 0)";
Logging::Log(deviceName, Logging::LogLevel::Information);
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

    std::vector<const char*> deviceExts(s_deviceExtensions.begin(), s_deviceExtensions.end());

    // Check for HDR device extensions
    uint32_t devExtCount = 0;
    vkEnumerateDeviceExtensionProperties(m_physicalDevice, nullptr, &devExtCount, nullptr);
    std::vector<VkExtensionProperties> devExts(devExtCount);
    vkEnumerateDeviceExtensionProperties(m_physicalDevice, nullptr, &devExtCount, devExts.data());

    for (const auto& hdrExt : s_deviceExtensionsHdr)
    {
        for (const auto& avail : devExts)
        {
            if (strcmp(avail.extensionName, hdrExt) == 0)
            {
                deviceExts.push_back(hdrExt);
                m_hdrMetadataSupported = true;
                Logging::Log(std::string("Vulkan: HDR device extension enabled: ") + hdrExt, Logging::LogLevel::Information);
                break;
            }
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
    createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExts.size());
    createInfo.ppEnabledExtensionNames = deviceExts.data();

    if (vkCreateDevice(m_physicalDevice, &createInfo, nullptr, &m_device) != VK_SUCCESS)
        throw std::runtime_error("Failed to create Vulkan device");

    vkGetDeviceQueue(m_device, m_graphicsQueueFamily, 0, &m_graphicsQueue);
}

void VkGraphicsBackend::InitAllocator()
{
    VmaAllocatorCreateInfo allocInfo{};
    allocInfo.physicalDevice = m_physicalDevice;
    allocInfo.device = m_device;
    allocInfo.instance = m_instance;
    allocInfo.vulkanApiVersion = VK_API_VERSION_1_3;

    if (vmaCreateAllocator(&allocInfo, &m_allocator) != VK_SUCCESS)
        throw std::runtime_error("Failed to create VMA allocator");
}

void VkGraphicsBackend::InitCommandPool()
{
    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = m_graphicsQueueFamily;

    if (vkCreateCommandPool(m_device, &poolInfo, nullptr, &m_commandPool) != VK_SUCCESS)
        throw std::runtime_error("Failed to create command pool");

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = m_commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;

    if (vkAllocateCommandBuffers(m_device, &allocInfo, &m_commandBuffer) != VK_SUCCESS)
        throw std::runtime_error("Failed to allocate command buffer");

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    vkCreateFence(m_device, &fenceInfo, nullptr, &m_frameFence);
}

//====================================================================
// Swap Chain
//====================================================================

void VkGraphicsBackend::SetSurface(VkSurfaceKHR surface, int width, int height)
{
    if (m_surface != VK_NULL_HANDLE)
    {
        DestroySwapChain();
        m_surface = VK_NULL_HANDLE;
    }
    m_surface = surface;
    m_surfaceWidth = width;
    m_surfaceHeight = height;
    CreateSwapChain();
}

void VkGraphicsBackend::CreateSwapChain()
{
    if (!m_surface)
        return;

    VkSurfaceCapabilitiesKHR caps{};
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_physicalDevice, m_surface, &caps);

    uint32_t formatCount = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(m_physicalDevice, m_surface, &formatCount, nullptr);
    std::vector<VkSurfaceFormatKHR> formats(formatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(m_physicalDevice, m_surface, &formatCount, formats.data());

    // Default to SDR (standard 8-bit sRGB)
    m_swapChainFormat = VK_FORMAT_B8G8R8A8_UNORM;
    m_swapChainColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
    m_hdrEnabled = false;

    // Try to find an HDR format, preferring FP16 scRGB linear
    // scRGB: pixel value 1.0 = SDR white (80 nits), values >1.0 for HDR highlights
    for (auto& fmt : formats)
    {
        if (fmt.format == VK_FORMAT_R16G16B16A16_SFLOAT &&
            fmt.colorSpace == VK_COLOR_SPACE_EXTENDED_SRGB_LINEAR_EXT)
        {
            m_swapChainFormat = VK_FORMAT_R16G16B16A16_SFLOAT;
            m_swapChainColorSpace = VK_COLOR_SPACE_EXTENDED_SRGB_LINEAR_EXT;
            m_hdrEnabled = true;
            Logging::Log("Vulkan: HDR FP16 scRGB swap chain selected", Logging::LogLevel::Information);
            break;
        }
    }

    // Fallback: try 10-bit HDR10 PQ
    if (!m_hdrEnabled)
    {
        for (auto& fmt : formats)
        {
            if (fmt.format == VK_FORMAT_A2R10G10B10_UNORM_PACK32 &&
                fmt.colorSpace == VK_COLOR_SPACE_HDR10_ST2084_EXT)
            {
                m_swapChainFormat = VK_FORMAT_A2R10G10B10_UNORM_PACK32;
                m_swapChainColorSpace = VK_COLOR_SPACE_HDR10_ST2084_EXT;
                m_hdrEnabled = true;
                Logging::Log("Vulkan: HDR10 PQ swap chain selected", Logging::LogLevel::Information);
                break;
            }
        }
    }

    // Fallback: SDR
    if (!m_hdrEnabled)
    {
        for (auto& fmt : formats)
        {
            if (fmt.format == VK_FORMAT_B8G8R8A8_UNORM &&
                fmt.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
            {
                m_swapChainFormat = fmt.format;
                m_swapChainColorSpace = fmt.colorSpace;
                Logging::Log("Vulkan: SDR swap chain selected", Logging::LogLevel::Information);
                break;
            }
        }
    }

    m_swapChainExtent = caps.currentExtent;
    if (m_swapChainExtent.width == UINT32_MAX)
    {
        m_swapChainExtent.width = std::max(caps.minImageExtent.width,
            std::min(caps.maxImageExtent.width, static_cast<uint32_t>(m_surfaceWidth)));
        m_swapChainExtent.height = std::max(caps.minImageExtent.height,
            std::min(caps.maxImageExtent.height, static_cast<uint32_t>(m_surfaceHeight)));
    }

    uint32_t imageCount = std::min(caps.minImageCount + 1, caps.maxImageCount > 0 ? caps.maxImageCount : 3);

    VkSwapchainCreateInfoKHR swapInfo{};
    swapInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapInfo.surface = m_surface;
    swapInfo.minImageCount = imageCount;
    swapInfo.imageFormat = m_swapChainFormat;
    swapInfo.imageColorSpace = m_swapChainColorSpace;
    swapInfo.imageExtent = m_swapChainExtent;
    swapInfo.imageArrayLayers = 1;
    swapInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    swapInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapInfo.queueFamilyIndexCount = 1;
    swapInfo.pQueueFamilyIndices = &m_graphicsQueueFamily;
    swapInfo.preTransform = caps.currentTransform;
    swapInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapInfo.presentMode = VK_PRESENT_MODE_FIFO_KHR;
    swapInfo.clipped = VK_TRUE;

    if (vkCreateSwapchainKHR(m_device, &swapInfo, nullptr, &m_swapChain) != VK_SUCCESS)
        throw std::runtime_error("Failed to create swap chain");

    vkGetSwapchainImagesKHR(m_device, m_swapChain, &imageCount, nullptr);
    m_swapChainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(m_device, m_swapChain, &imageCount, m_swapChainImages.data());

    m_swapChainImageViews.resize(imageCount);
    for (uint32_t i = 0; i < imageCount; i++)
    {
        VkImageViewCreateInfo viewInfo{};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = m_swapChainImages[i];
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = m_swapChainFormat;
        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.layerCount = 1;
        vkCreateImageView(m_device, &viewInfo, nullptr, &m_swapChainImageViews[i]);
    }

    // Set HDR metadata if supported
    if (m_hdrEnabled && m_hdrMetadataSupported && m_vkSetHdrMetadataEXT)
    {
        VkHdrMetadataEXT hdrMetadata{};
        hdrMetadata.sType = VK_STRUCTURE_TYPE_HDR_METADATA_EXT;

        // SMPTE ST 2086 mastering display color volume
        // These are typical values for a 1000-nit HDR display
        hdrMetadata.displayPrimaryRed.x = 0.708f;
        hdrMetadata.displayPrimaryRed.y = 0.292f;
        hdrMetadata.displayPrimaryGreen.x = 0.170f;
        hdrMetadata.displayPrimaryGreen.y = 0.797f;
        hdrMetadata.displayPrimaryBlue.x = 0.131f;
        hdrMetadata.displayPrimaryBlue.y = 0.046f;
        hdrMetadata.whitePoint.x = 0.3127f;
        hdrMetadata.whitePoint.y = 0.3290f;
        hdrMetadata.maxLuminance = 1000.0f;
        hdrMetadata.minLuminance = 0.01f;

        // CTA-861.3 HDR static metadata
        hdrMetadata.maxContentLightLevel = 1000.0f;
        hdrMetadata.maxFrameAverageLightLevel = 400.0f;

        m_vkSetHdrMetadataEXT(m_device, 1, &m_swapChain, &hdrMetadata);
        Logging::Log("Vulkan: HDR metadata set (1000-nit display)", Logging::LogLevel::Information);
    }

    std::string chanMsg = "Swap chain created: " + std::to_string(m_swapChainExtent.width) + "x" +
        std::to_string(m_swapChainExtent.height) + (m_hdrEnabled ? " (HDR)" : " (SDR)");
    Logging::Log(chanMsg, Logging::LogLevel::Information);
}

void VkGraphicsBackend::DestroySwapChain()
{
    for (auto view : m_swapChainImageViews)
        vkDestroyImageView(m_device, view, nullptr);
    m_swapChainImageViews.clear();
    m_swapChainImages.clear();

    if (m_swapChain)
    {
        vkDestroySwapchainKHR(m_device, m_swapChain, nullptr);
        m_swapChain = VK_NULL_HANDLE;
    }
}

//====================================================================
// Frame Management
//====================================================================

void VkGraphicsBackend::BeginFrame()
{
    vkWaitForFences(m_device, 1, &m_frameFence, VK_TRUE, UINT64_MAX);
    vkResetFences(m_device, 1, &m_frameFence);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vkBeginCommandBuffer(m_commandBuffer, &beginInfo);
}

void VkGraphicsBackend::EndFrame()
{
    vkEndCommandBuffer(m_commandBuffer);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &m_commandBuffer;
    vkQueueSubmit(m_graphicsQueue, 1, &submitInfo, m_frameFence);
}

//====================================================================
// State Commands
//====================================================================

void VkGraphicsBackend::SetViewport(int x, int y, int width, int height)
{
    m_viewport.x = static_cast<float>(x);
    m_viewport.y = static_cast<float>(y);
    m_viewport.width = static_cast<float>(width);
    m_viewport.height = static_cast<float>(height);
    m_viewport.minDepth = 0.0f;
    m_viewport.maxDepth = 1.0f;
    vkCmdSetViewport(m_commandBuffer, 0, 1, &m_viewport);
}

void VkGraphicsBackend::ClearColor(float r, float g, float b, float a)
{
    m_clearColor[0] = r;
    m_clearColor[1] = g;
    m_clearColor[2] = b;
    m_clearColor[3] = a;
}

void VkGraphicsBackend::Clear()
{
    VkClearAttachment clearAttach{};
    clearAttach.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    clearAttach.colorAttachment = 0;
    memcpy(clearAttach.clearValue.color.float32, m_clearColor, sizeof(m_clearColor));

    VkClearRect clearRect{};
    clearRect.rect.offset = {0, 0};
    clearRect.rect.extent = {static_cast<uint32_t>(m_viewport.width),
                             static_cast<uint32_t>(m_viewport.height)};
    clearRect.layerCount = 1;

    vkCmdClearAttachments(m_commandBuffer, 1, &clearAttach, 1, &clearRect);
}

void VkGraphicsBackend::SetBlendMode(BlendFactor, BlendFactor, BlendFactor, BlendFactor, bool) {}

void VkGraphicsBackend::DrawArrays(PrimitiveType mode, int first, int count)
{
    vkCmdDraw(m_commandBuffer, static_cast<uint32_t>(count), 1,
              static_cast<uint32_t>(first), 0);
}

void VkGraphicsBackend::DrawElements(PrimitiveType mode, int count,
                                     IndexType indexType, const void* indices)
{
    if (indices)
    {
        vkCmdDrawIndexed(m_commandBuffer, static_cast<uint32_t>(count), 1,
                         static_cast<uint32_t>(reinterpret_cast<uintptr_t>(indices) / sizeof(uint32_t)),
                         0, 0);
    }
    else
    {
        vkCmdDrawIndexed(m_commandBuffer, static_cast<uint32_t>(count), 1,
                         m_commandBuffer == VK_NULL_HANDLE ? 0 : 0,
                         0, 0);
    }
}

void VkGraphicsBackend::SetLineWidth(float) {}

void VkGraphicsBackend::SetLineSmoothing(bool) {}

void VkGraphicsBackend::SetScissor(int x, int y, int width, int height, bool enable)
{
    if (enable)
    {
        m_scissor.offset = {x, y};
        m_scissor.extent = {static_cast<uint32_t>(width), static_cast<uint32_t>(height)};
        vkCmdSetScissor(m_commandBuffer, 0, 1, &m_scissor);
    }
}

void VkGraphicsBackend::SetConstantVertexAttrib4f(unsigned int, float v0, float v1, float v2, float v3)
{
    m_constantAttrib[0] = v0;
    m_constantAttrib[1] = v1;
    m_constantAttrib[2] = v2;
    m_constantAttrib[3] = v3;
}

void VkGraphicsBackend::EnableVertexAttribArray(unsigned int, bool) {}

void VkGraphicsBackend::BindDefaultFramebuffer()
{
    // Vulkan binds to swap chain via render pass begin
}

void VkGraphicsBackend::BindDrawFramebufferRaw(uint32_t) {}
void VkGraphicsBackend::BindReadFramebufferRaw(uint32_t) {}
auto VkGraphicsBackend::GetDrawFramebufferBindingRaw() -> uint32_t { return 0; }
auto VkGraphicsBackend::GetReadFramebufferBindingRaw() -> uint32_t { return 0; }
void VkGraphicsBackend::CopyFramebufferToTexture(int, int, int, int, int, int, int, int) {}
void VkGraphicsBackend::EnsureDefaultDrawBuffers() {}
void VkGraphicsBackend::SetBoundTextureWrap(SamplerWrap, SamplerWrap) {}

auto VkGraphicsBackend::GetShaderLanguageVersion() const -> GlslVersion
{
    return {4, 6};
}

//====================================================================
// GLSL to SPIR-V Compilation
//====================================================================

auto VkGraphicsBackend::CompileGLSLtoSPIRV(VkShaderStageFlagBits stage,
                                           const char* source,
                                           VkShaderModule& outModule,
                                           VkDevice device) -> bool
{
    // Use glslangValidator via external process for now.
    // In production, glslang library would be linked directly.
    // For runtime compilation, we shell out to glslc.
    //
    // Write source to temp file, compile, read SPIR-V binary.

    auto tempSrcFile = "/tmp/projectm_shader.glsl";
    auto tempSpvFile = "/tmp/projectm_shader.spv";

    // Write source to temp file
    {
        FILE* f = fopen(tempSrcFile, "w");
        if (!f)
            return false;
        fputs("#version 450\n\n", f);
        fputs(source, f);
        fclose(f);
    }

    // Determine shader stage
    const char* stageOpt = "";
    switch (stage)
    {
        case VK_SHADER_STAGE_VERTEX_BIT: stageOpt = "vert"; break;
        case VK_SHADER_STAGE_FRAGMENT_BIT: stageOpt = "frag"; break;
        case VK_SHADER_STAGE_COMPUTE_BIT: stageOpt = "comp"; break;
        default: return false;
    }

    // Compile with glslangValidator
    char cmd[1024];
    snprintf(cmd, sizeof(cmd),
             "glslangValidator -V --target-env vulkan1.3 -S %s -o %s %s 2>/dev/null",
             stageOpt, tempSpvFile, tempSrcFile);

    int ret = system(cmd);
    if (ret != 0)
    {
Logging::Log("Failed to compile GLSL to SPIR-V", Logging::LogLevel::Error);
        return false;
    }

    // Read compiled SPIR-V binary
    FILE* f = fopen(tempSpvFile, "rb");
    if (!f)
        return false;

    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);

    std::vector<uint32_t> spirv(size / sizeof(uint32_t));
    fread(spirv.data(), 1, size, f);
    fclose(f);

    // Create shader module
    VkShaderModuleCreateInfo moduleInfo{};
    moduleInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    moduleInfo.codeSize = spirv.size() * sizeof(uint32_t);
    moduleInfo.pCode = spirv.data();

    if (vkCreateShaderModule(device, &moduleInfo, nullptr, &outModule) != VK_SUCCESS)
        return false;

    return true;
}

//====================================================================
// Resource: CreateTexture
//====================================================================

auto VkGraphicsBackend::CreateTexture(std::string name, int width, int height,
                                      bool isUserTexture) -> std::shared_ptr<Texture>
{
    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
    imageInfo.extent = {static_cast<uint32_t>(width), static_cast<uint32_t>(height), 1};
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT |
                      VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    VmaAllocationCreateInfo allocInfo{};
    allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

    VkImage image;
    VmaAllocation allocation;
    if (vmaCreateImage(m_allocator, &imageInfo, &allocInfo, &image, &allocation, nullptr) != VK_SUCCESS)
    {
Logging::Log("Failed to create Vulkan texture: " + name, Logging::LogLevel::Error);
        throw std::runtime_error("Vulkan: CreateTexture failed");
    }

    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.layerCount = 1;

    VkImageView view;
    vkCreateImageView(m_device, &viewInfo, nullptr, &view);

    return std::make_shared<VkTextureAdapter>(
        std::move(name), m_device, m_allocator,
        image, allocation, view, VK_FORMAT_R8G8B8A8_UNORM,
        width, height, 1, isUserTexture);
}

auto VkGraphicsBackend::CreateTexture(std::string name, int width, int height, int depth,
                                      TextureFormat internalFormat) -> std::shared_ptr<Texture>
{
    auto vkFormat = ToVkFormat(internalFormat);
    bool is3D = (depth > 1);

    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = is3D ? VK_IMAGE_TYPE_3D : VK_IMAGE_TYPE_2D;
    imageInfo.format = vkFormat;
    imageInfo.extent = {static_cast<uint32_t>(width), static_cast<uint32_t>(height), static_cast<uint32_t>(std::max(1, depth))};
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    VmaAllocationCreateInfo allocInfo{};
    allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

    VkImage image;
    VmaAllocation allocation;
    if (vmaCreateImage(m_allocator, &imageInfo, &allocInfo, &image, &allocation, nullptr) != VK_SUCCESS)
    {
Logging::Log("Failed to create Vulkan texture: " + name, Logging::LogLevel::Error);
        throw std::runtime_error("Vulkan: CreateTexture failed");
    }

    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = image;
    viewInfo.viewType = is3D ? VK_IMAGE_VIEW_TYPE_3D : VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = vkFormat;
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.layerCount = 1;

    VkImageView view;
    vkCreateImageView(m_device, &viewInfo, nullptr, &view);

    return std::make_shared<VkTextureAdapter>(
        std::move(name), m_device, m_allocator,
        image, allocation, view, vkFormat,
        width, height, depth, false);
}

//====================================================================
// Resource: CreateFramebuffer
//====================================================================

auto VkGraphicsBackend::CreateFramebuffer(int count) -> std::shared_ptr<Framebuffer>
{
    return std::make_shared<VkFramebufferAdapter>(count, m_device, m_allocator, this);
}

//====================================================================
// Resource: CreateShaderProgram
//====================================================================

auto VkGraphicsBackend::CreateShaderProgram(std::string_view vertexSrc,
                                            std::string_view fragmentSrc) -> std::shared_ptr<ShaderProgram>
{
    VkShaderModule vertModule, fragModule;

    if (!CompileGLSLtoSPIRV(VK_SHADER_STAGE_VERTEX_BIT, vertexSrc.data(), vertModule, m_device))
        throw std::runtime_error("Vulkan: Failed to compile vertex shader");

    if (!CompileGLSLtoSPIRV(VK_SHADER_STAGE_FRAGMENT_BIT, fragmentSrc.data(), fragModule, m_device))
    {
        vkDestroyShaderModule(m_device, vertModule, nullptr);
        throw std::runtime_error("Vulkan: Failed to compile fragment shader");
    }

    // Create pipeline layout (empty for now)
    VkPipelineLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

    VkPipelineLayout pipelineLayout;
    vkCreatePipelineLayout(m_device, &layoutInfo, nullptr, &pipelineLayout);

    // Minimal graphics pipeline
    VkPipelineShaderStageCreateInfo stages[2]{};
    stages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    stages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
    stages[0].module = vertModule;
    stages[0].pName = "main";
    stages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    stages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    stages[1].module = fragModule;
    stages[1].pName = "main";

    VkPipelineVertexInputStateCreateInfo vertexInput{};
    vertexInput.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.scissorCount = 1;

    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.lineWidth = 1.0f;

    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkPipelineColorBlendAttachmentState colorBlend{};
    colorBlend.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                                VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlend.blendEnable = VK_FALSE;

    VkPipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlend;

    VkDynamicState dynamicStates[] = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
    VkPipelineDynamicStateCreateInfo dynamicState{};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = 2;
    dynamicState.pDynamicStates = dynamicStates;

    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = stages;
    pipelineInfo.pVertexInputState = &vertexInput;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = &dynamicState;
    pipelineInfo.layout = pipelineLayout;
    pipelineInfo.renderPass = VK_NULL_HANDLE; // Will be set when bound to framebuffer
    pipelineInfo.subpass = 0;

    VkPipeline pipeline;
    if (vkCreateGraphicsPipelines(m_device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline) != VK_SUCCESS)
    {
        vkDestroyPipelineLayout(m_device, pipelineLayout, nullptr);
        vkDestroyShaderModule(m_device, vertModule, nullptr);
        vkDestroyShaderModule(m_device, fragModule, nullptr);
        throw std::runtime_error("Vulkan: Failed to create graphics pipeline");
    }

    return std::make_shared<VkShaderProgramAdapter>(
        pipeline, pipelineLayout,
        std::vector<VkShaderModule>{vertModule, fragModule},
        m_device);
}

//====================================================================
// Resource: CreateSampler
//====================================================================

auto VkGraphicsBackend::CreateSampler(SamplerWrap wrapMode, SamplerFilter filterMode) -> std::shared_ptr<Sampler>
{
    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = ToVkFilter(filterMode);
    samplerInfo.minFilter = ToVkFilter(filterMode);
    samplerInfo.addressModeU = ToVkSamplerAddressMode(wrapMode);
    samplerInfo.addressModeV = ToVkSamplerAddressMode(wrapMode);
    samplerInfo.addressModeW = ToVkSamplerAddressMode(wrapMode);
    samplerInfo.maxLod = VK_LOD_CLAMP_NONE;

    VkSampler sampler;
    if (vkCreateSampler(m_device, &samplerInfo, nullptr, &sampler) != VK_SUCCESS)
        throw std::runtime_error("Vulkan: Failed to create sampler");

    return std::make_shared<VkSamplerAdapter>(sampler, m_device, wrapMode, filterMode);
}

//====================================================================
// Resource: CreateMesh
//====================================================================

auto VkGraphicsBackend::CreateMesh() -> std::shared_ptr<VertexMesh>
{
    return std::make_shared<VkVertexMeshAdapter>(m_device, m_allocator, this);
}

//====================================================================
//=== VkTextureAdapter ================================================
//====================================================================

VkGraphicsBackend::VkTextureAdapter::VkTextureAdapter(std::string name, VkDevice device,
                                                       VmaAllocator allocator,
                                                       VkImage image, VmaAllocation allocation,
                                                       VkImageView view,
                                                       VkFormat format,
                                                       int width, int height, int depth,
                                                       bool isUserTexture)
    : m_name(std::move(name))
    , m_device(device)
    , m_allocator(allocator)
    , m_image(image)
    , m_allocation(allocation)
    , m_view(view)
    , m_format(format)
    , m_width(width)
    , m_height(height)
    , m_depth(depth)
    , m_isUserTexture(isUserTexture)
{
}

VkGraphicsBackend::VkTextureAdapter::~VkTextureAdapter()
{
    if (m_view)
        vkDestroyImageView(m_device, m_view, nullptr);
    if (m_allocation)
        vmaDestroyImage(m_allocator, m_image, m_allocation);
}

void VkGraphicsBackend::VkTextureAdapter::Bind(int, const std::shared_ptr<Backend::Sampler>&)
{
    // Descriptor set binding happens during pipeline setup
}

void VkGraphicsBackend::VkTextureAdapter::Unbind(int) {}

void VkGraphicsBackend::VkTextureAdapter::Update(const void* data)
{
    if (!data || !m_image)
        return;

    // For user textures, upload via staging buffer
    size_t pixelSize = 4; // RGBA8
    switch (m_format)
    {
        case VK_FORMAT_R8_UNORM: pixelSize = 1; break;
        case VK_FORMAT_R8G8_UNORM: pixelSize = 2; break;
        case VK_FORMAT_R8G8B8_UNORM: pixelSize = 3; break;
        case VK_FORMAT_R8G8B8A8_UNORM: pixelSize = 4; break;
        case VK_FORMAT_R16G16B16A16_SFLOAT: pixelSize = 8; break;
        case VK_FORMAT_R32G32B32A32_SFLOAT: pixelSize = 16; break;
        default: pixelSize = 4; break;
    }

    VkDeviceSize bufferSize = static_cast<VkDeviceSize>(m_width) * m_height * std::max(1, m_depth) * pixelSize;

    VkBuffer stagingBuffer;
    VmaAllocation stagingAlloc;
    VmaAllocationInfo stagingAllocInfo{};

    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = bufferSize;
    bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

    VmaAllocationCreateInfo allocCreateInfo{};
    allocCreateInfo.usage = VMA_MEMORY_USAGE_CPU_ONLY;
    allocCreateInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;

    vmaCreateBuffer(m_allocator, &bufferInfo, &allocCreateInfo,
                    &stagingBuffer, &stagingAlloc, &stagingAllocInfo);
    memcpy(stagingAllocInfo.pMappedData, data, static_cast<size_t>(bufferSize));
    vmaFlushAllocation(m_allocator, stagingAlloc, 0, bufferSize);

    // Transition image, copy buffer, transition back
    // (simplified - full implementation needs a one-time command buffer)
    // For now this is a simplified copy path
    //
    // In production, this would use a dedicated transfer queue and
    // proper pipeline barriers.

    vmaDestroyBuffer(m_allocator, stagingBuffer, stagingAlloc);
}

//====================================================================
//=== VkFramebufferAdapter ============================================
//====================================================================

VkGraphicsBackend::VkFramebufferAdapter::VkFramebufferAdapter(int count, VkDevice device,
                                                               VmaAllocator allocator,
                                                               VkGraphicsBackend* backend)
    : m_device(device)
    , m_allocator(allocator)
    , m_backend(backend)
    , m_count(count)
{
    m_buffers.resize(count);
}

VkGraphicsBackend::VkFramebufferAdapter::~VkFramebufferAdapter()
{
    for (auto& buf : m_buffers)
    {
        if (buf.framebuffer)
            vkDestroyFramebuffer(m_device, buf.framebuffer, nullptr);
        if (buf.renderPass)
            vkDestroyRenderPass(m_device, buf.renderPass, nullptr);
    }
}

void VkGraphicsBackend::VkFramebufferAdapter::Bind(int index)
{
    if (index < 0 || index >= m_count)
        return;

    if (m_buffers[index].framebuffer == VK_NULL_HANDLE)
        CreateVkFramebuffer(index);

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = m_buffers[index].renderPass;
    renderPassInfo.framebuffer = m_buffers[index].framebuffer;
    renderPassInfo.renderArea.extent = {static_cast<uint32_t>(m_width), static_cast<uint32_t>(m_height)};

    VkClearValue clearValue{};
    clearValue.color = {{0, 0, 0, 1}};
    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &clearValue;

    vkCmdBeginRenderPass(m_backend->m_commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
}

void VkGraphicsBackend::VkFramebufferAdapter::BindRead(int) {}
void VkGraphicsBackend::VkFramebufferAdapter::BindDraw(int index) { Bind(index); }

void VkGraphicsBackend::VkFramebufferAdapter::Unbind()
{
    vkCmdEndRenderPass(m_backend->m_commandBuffer);
}

auto VkGraphicsBackend::VkFramebufferAdapter::SetSize(int width, int height) -> bool
{
    if (width == m_width && height == m_height)
        return false;
    m_width = width;
    m_height = height;
    // Recreate framebuffers for new size
    for (int i = 0; i < m_count; i++)
        DestroyVkFramebuffer(i);
    return true;
}

auto VkGraphicsBackend::VkFramebufferAdapter::GetColorAttachmentTexture(int idx, int attIdx) const -> std::shared_ptr<Backend::Texture>
{
    if (idx < 0 || idx >= m_count || attIdx < 0)
        return nullptr;
    auto& buf = m_buffers[idx];
    if (attIdx >= static_cast<int>(buf.colorTextures.size()))
        return nullptr;
    return buf.colorTextures[attIdx];
}

void VkGraphicsBackend::VkFramebufferAdapter::CreateColorAttachment(int idx, int attIdx)
{
    if (idx < 0 || idx >= m_count)
        return;

    auto& buf = m_buffers[idx];
    if (attIdx >= static_cast<int>(buf.colorTextures.size()))
    {
        buf.colorTextures.resize(attIdx + 1);
        buf.colorMasks.resize(attIdx + 1, false);
    }

    auto tex = m_backend->CreateTexture(
        "fb" + std::to_string(idx) + "_a" + std::to_string(attIdx),
        std::max(1, m_width), std::max(1, m_height), false);
    buf.colorTextures[attIdx] = tex;
}

void VkGraphicsBackend::VkFramebufferAdapter::RemoveColorAttachment(int idx, int attIdx)
{
    if (idx < 0 || idx >= m_count)
        return;
    auto& buf = m_buffers[idx];
    if (attIdx < static_cast<int>(buf.colorTextures.size()))
        buf.colorTextures[attIdx].reset();
}

void VkGraphicsBackend::VkFramebufferAdapter::SetAttachment(int idx, int attIdx, const std::shared_ptr<TextureAttachment>& attachment)
{
    if (idx < 0 || idx >= m_count)
        return;
    auto& buf = m_buffers[idx];
    if (attIdx >= static_cast<int>(buf.colorTextures.size()))
    {
        buf.colorTextures.resize(attIdx + 1);
        buf.colorMasks.resize(attIdx + 1, false);
    }
    if (attachment)
        buf.colorTextures[attIdx] = attachment->Texture();
    else
        buf.colorTextures[attIdx].reset();
}

void VkGraphicsBackend::VkFramebufferAdapter::MaskDrawBuffer(int bufIdx, bool masked)
{
    if (bufIdx < 0 || bufIdx >= m_count)
        return;
    if (bufIdx < static_cast<int>(m_buffers.size()))
    {
        if (bufIdx >= static_cast<int>(m_buffers[bufIdx].colorMasks.size()))
            m_buffers[bufIdx].colorMasks.resize(bufIdx + 1, false);
        m_buffers[bufIdx].colorMasks[bufIdx] = masked;
    }
}

void VkGraphicsBackend::VkFramebufferAdapter::CreateVkFramebuffer(int index)
{
    if (m_width <= 0 || m_height <= 0)
        return;

    auto& buf = m_buffers[index];

    std::vector<VkAttachmentDescription> attachments;
    std::vector<VkAttachmentReference> colorRefs;
    VkImageView views[8]{};
    uint32_t viewCount = 0;

    for (size_t i = 0; i < buf.colorTextures.size() && viewCount < 8; i++)
    {
        if (!buf.colorTextures[i])
            continue;

        auto* vkTex = dynamic_cast<VkTextureAdapter*>(buf.colorTextures[i].get());
        if (!vkTex || vkTex->Image() == VK_NULL_HANDLE)
            continue;

        VkAttachmentDescription att{};
        att.format = VK_FORMAT_R8G8B8A8_UNORM;
        att.samples = VK_SAMPLE_COUNT_1_BIT;
        att.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
        att.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        att.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        att.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        attachments.push_back(att);

        VkAttachmentReference ref{};
        ref.attachment = static_cast<uint32_t>(i);
        ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        colorRefs.push_back(ref);

        views[viewCount++] = vkTex->View();
    }

    if (attachments.empty() || viewCount == 0)
        return;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = static_cast<uint32_t>(colorRefs.size());
    subpass.pColorAttachments = colorRefs.data();

    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    renderPassInfo.pAttachments = attachments.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;

    vkCreateRenderPass(m_device, &renderPassInfo, nullptr, &buf.renderPass);

    VkFramebufferCreateInfo fbInfo{};
    fbInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    fbInfo.renderPass = buf.renderPass;
    fbInfo.attachmentCount = viewCount;
    fbInfo.pAttachments = views;
    fbInfo.width = static_cast<uint32_t>(m_width);
    fbInfo.height = static_cast<uint32_t>(m_height);
    fbInfo.layers = 1;

    vkCreateFramebuffer(m_device, &fbInfo, nullptr, &buf.framebuffer);
}

void VkGraphicsBackend::VkFramebufferAdapter::DestroyVkFramebuffer(int index)
{
    auto& buf = m_buffers[index];
    if (buf.framebuffer)
    {
        vkDestroyFramebuffer(m_device, buf.framebuffer, nullptr);
        buf.framebuffer = VK_NULL_HANDLE;
    }
    if (buf.renderPass)
    {
        vkDestroyRenderPass(m_device, buf.renderPass, nullptr);
        buf.renderPass = VK_NULL_HANDLE;
    }
}

//====================================================================
//=== VkShaderProgramAdapter =========================================
//====================================================================

VkGraphicsBackend::VkShaderProgramAdapter::VkShaderProgramAdapter(
    VkPipeline pipeline, VkPipelineLayout pipelineLayout,
    std::vector<VkShaderModule> modules, VkDevice device)
    : m_pipeline(pipeline)
    , m_pipelineLayout(pipelineLayout)
    , m_modules(std::move(modules))
    , m_device(device)
{
}

VkGraphicsBackend::VkShaderProgramAdapter::~VkShaderProgramAdapter()
{
    if (m_pipeline)
        vkDestroyPipeline(m_device, m_pipeline, nullptr);
    if (m_pipelineLayout)
        vkDestroyPipelineLayout(m_device, m_pipelineLayout, nullptr);
    for (auto mod : m_modules)
        vkDestroyShaderModule(m_device, mod, nullptr);
}

void VkGraphicsBackend::VkShaderProgramAdapter::Bind()
{
    // Pipeline binding handled by backend during render pass
}

void VkGraphicsBackend::VkShaderProgramAdapter::Unbind() {}

auto VkGraphicsBackend::VkShaderProgramAdapter::Validate(std::string& msg) const -> bool
{
    msg.clear();
    return m_pipeline != VK_NULL_HANDLE;
}

void VkGraphicsBackend::VkShaderProgramAdapter::SetUniformFloat(const char*, float) {}
void VkGraphicsBackend::VkShaderProgramAdapter::SetUniformInt(const char*, int) {}
void VkGraphicsBackend::VkShaderProgramAdapter::SetUniformFloat2(const char*, const glm::vec2&) {}
void VkGraphicsBackend::VkShaderProgramAdapter::SetUniformInt2(const char*, const glm::ivec2&) {}
void VkGraphicsBackend::VkShaderProgramAdapter::SetUniformFloat3(const char*, const glm::vec3&) {}
void VkGraphicsBackend::VkShaderProgramAdapter::SetUniformInt3(const char*, const glm::ivec3&) {}
void VkGraphicsBackend::VkShaderProgramAdapter::SetUniformFloat4(const char*, const glm::vec4&) {}
void VkGraphicsBackend::VkShaderProgramAdapter::SetUniformInt4(const char*, const glm::ivec4&) {}
void VkGraphicsBackend::VkShaderProgramAdapter::SetUniformMat3x4(const char*, const glm::mat3x4&) {}
void VkGraphicsBackend::VkShaderProgramAdapter::SetUniformMat4x4(const char*, const glm::mat4x4&) {}

//====================================================================
//=== VkSamplerAdapter ===============================================
//====================================================================

VkGraphicsBackend::VkSamplerAdapter::VkSamplerAdapter(VkSampler sampler, VkDevice device,
                                                       SamplerWrap wrapMode, SamplerFilter filterMode)
    : m_sampler(sampler)
    , m_device(device)
    , m_wrapMode(wrapMode)
    , m_filterMode(filterMode)
{
}

VkGraphicsBackend::VkSamplerAdapter::~VkSamplerAdapter()
{
    if (m_sampler)
        vkDestroySampler(m_device, m_sampler, nullptr);
}

void VkGraphicsBackend::VkSamplerAdapter::Bind(unsigned int) {}
void VkGraphicsBackend::VkSamplerAdapter::Unbind(unsigned int) {}

void VkGraphicsBackend::VkSamplerAdapter::WrapMode(SamplerWrap mode)
{
    // Recreate sampler with new wrap mode
    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = ToVkFilter(m_filterMode);
    samplerInfo.minFilter = ToVkFilter(m_filterMode);
    samplerInfo.addressModeU = ToVkSamplerAddressMode(mode);
    samplerInfo.addressModeV = ToVkSamplerAddressMode(mode);
    samplerInfo.addressModeW = ToVkSamplerAddressMode(mode);
    samplerInfo.maxLod = VK_LOD_CLAMP_NONE;

    VkSampler newSampler;
    vkCreateSampler(m_device, &samplerInfo, nullptr, &newSampler);
    if (m_sampler)
        vkDestroySampler(m_device, m_sampler, nullptr);
    m_sampler = newSampler;
    m_wrapMode = mode;
}

void VkGraphicsBackend::VkSamplerAdapter::FilterMode(SamplerFilter mode)
{
    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = ToVkFilter(mode);
    samplerInfo.minFilter = ToVkFilter(mode);
    samplerInfo.addressModeU = ToVkSamplerAddressMode(m_wrapMode);
    samplerInfo.addressModeV = ToVkSamplerAddressMode(m_wrapMode);
    samplerInfo.addressModeW = ToVkSamplerAddressMode(m_wrapMode);
    samplerInfo.maxLod = VK_LOD_CLAMP_NONE;

    VkSampler newSampler;
    vkCreateSampler(m_device, &samplerInfo, nullptr, &newSampler);
    if (m_sampler)
        vkDestroySampler(m_device, m_sampler, nullptr);
    m_sampler = newSampler;
    m_filterMode = mode;
}

//====================================================================
//=== VkVertexMeshAdapter ============================================
//====================================================================

VkGraphicsBackend::VkVertexMeshAdapter::VkVertexMeshAdapter(VkDevice device, VmaAllocator allocator,
                                                             VkGraphicsBackend* backend)
    : m_device(device)
    , m_allocator(allocator)
    , m_backend(backend)
{
}

VkGraphicsBackend::VkVertexMeshAdapter::~VkVertexMeshAdapter()
{
    if (m_indexAllocation)
        vmaDestroyBuffer(m_allocator, m_indexBuffer, m_indexAllocation);
    if (m_vertexAllocation)
        vmaDestroyBuffer(m_allocator, m_vertexBuffer, m_vertexAllocation);
}

void VkGraphicsBackend::VkVertexMeshAdapter::Bind()
{
    // Bind vertex and index buffers to command buffer
    if (m_vertexBuffer != VK_NULL_HANDLE)
    {
        VkDeviceSize offset = 0;
        vkCmdBindVertexBuffers(m_backend->m_commandBuffer, 0, 1, &m_vertexBuffer, &offset);
    }
    if (m_indexBuffer != VK_NULL_HANDLE)
    {
        vkCmdBindIndexBuffer(m_backend->m_commandBuffer, m_indexBuffer, 0, VK_INDEX_TYPE_UINT32);
    }
}

void VkGraphicsBackend::VkVertexMeshAdapter::Unbind() {}

void VkGraphicsBackend::VkVertexMeshAdapter::Draw()
{
    // Draw is called by the pipeline code; actual draw calls happen via DrawArrays/DrawElements
}

void VkGraphicsBackend::VkVertexMeshAdapter::Update()
{
    // Mesh data upload - called when vertex/index data is modified
}

} // namespace Vulkan
} // namespace Backend
} // namespace Renderer
} // namespace libprojectM

#endif // PROJECTM_ENABLE_VULKAN
