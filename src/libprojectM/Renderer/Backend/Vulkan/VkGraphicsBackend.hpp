#pragma once

#include "Renderer/Backend/GraphicsBackend.hpp"
#include "Renderer/Backend/Resources.hpp"

#include <memory>
#include <string>

#ifdef PROJECTM_ENABLE_VULKAN

#include <vulkan/vulkan.h>

namespace libprojectM {
namespace Renderer {
namespace Backend {
namespace Vulkan {

/**
 * @brief Vulkan implementation of the GraphicsBackend interface.
 *
 * Supports HDR output via VK_EXT_hdr_metadata and HDR-capable swap chains.
 */
class VkGraphicsBackend : public GraphicsBackend
{
public:
    VkGraphicsBackend();
    ~VkGraphicsBackend() override;

    auto Name() const -> std::string_view override { return "Vulkan"; }

    void SetViewport(int x, int y, int width, int height) override;
    void ClearColor(float r, float g, float b, float a) override;
    void Clear() override;

    void SetBlendMode(BlendFactor srcRgb, BlendFactor dstRgb,
                      BlendFactor srcAlpha, BlendFactor dstAlpha,
                      bool enable) override;

    void DrawArrays(PrimitiveType mode, int first, int count) override;
    void DrawElements(PrimitiveType mode, int count,
                      IndexType indexType, const void* indices) override;

    void SetLineWidth(float width) override;
    void SetLineSmoothing(bool enable) override;
    void SetScissor(int x, int y, int width, int height, bool enable) override;

    void SetConstantVertexAttrib4f(unsigned int index,
                                   float v0, float v1, float v2, float v3) override;
    void EnableVertexAttribArray(unsigned int index, bool enable) override;

    void BindDefaultFramebuffer() override;
    void CopyFramebufferToTexture(int srcX0, int srcY0, int srcX1, int srcY1,
                                  int dstX0, int dstY0, int dstWidth, int dstHeight) override;

    auto GetShaderLanguageVersion() const -> GlslVersion override;

    auto CreateTexture(std::string name, int width, int height,
                       bool isUserTexture) -> std::shared_ptr<Texture> override;
    auto CreateTexture(std::string name, int width, int height, int depth,
                       TextureFormat internalFormat) -> std::shared_ptr<Texture> override;
    auto CreateFramebuffer(int count) -> std::shared_ptr<Framebuffer> override;
    auto CreateShaderProgram(std::string_view vertexSrc,
                             std::string_view fragmentSrc) -> std::shared_ptr<ShaderProgram> override;
    auto CreateSampler(SamplerWrap wrapMode, SamplerFilter filterMode) -> std::shared_ptr<Sampler> override;
    auto CreateMesh() -> std::shared_ptr<VertexMesh> override;

    //=== Vulkan-specific ===========================================
    auto GetInstance() const -> VkInstance { return m_instance; }
    auto GetDevice() const -> VkDevice { return m_device; }
    auto GetPhysicalDevice() const -> VkPhysicalDevice { return m_physicalDevice; }

private:
    void InitVulkan();
    void CleanupVulkan();
    void CreateInstance();
    void PickPhysicalDevice();
    void CreateDevice();

    VkInstance m_instance{VK_NULL_HANDLE};
    VkPhysicalDevice m_physicalDevice{VK_NULL_HANDLE};
    VkDevice m_device{VK_NULL_HANDLE};
    uint32_t m_graphicsQueueFamily{0};
    VkQueue m_graphicsQueue{VK_NULL_HANDLE};
};

} // namespace Vulkan
} // namespace Backend
} // namespace Renderer
} // namespace libprojectM

#endif // PROJECTM_ENABLE_VULKAN
