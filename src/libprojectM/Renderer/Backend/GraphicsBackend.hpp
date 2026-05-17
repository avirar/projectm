#pragma once

#include "Renderer/Backend/Types.hpp"

#include <memory>
#include <string_view>
#include <vector>

namespace libprojectM {
namespace Renderer {
namespace Backend {

class Texture;
class Framebuffer;
class ShaderProgram;
class VertexMesh;
class Sampler;

/**
 * @brief Abstract graphics backend interface.
 *
 * Implementations provide platform-specific GPU operations (OpenGL, Vulkan, etc.).
 */
class GraphicsBackend
{
public:
    virtual ~GraphicsBackend() = default;

    virtual auto Name() const -> std::string_view = 0;

    //=== Viewport / Clear ============================================
    virtual void SetViewport(int x, int y, int width, int height) = 0;
    virtual void ClearColor(float r, float g, float b, float a) = 0;
    virtual void Clear() = 0;

    //=== Blending ====================================================
    virtual void SetBlendMode(BlendFactor srcRgb, BlendFactor dstRgb,
                              BlendFactor srcAlpha, BlendFactor dstAlpha,
                              bool enable) = 0;

    //=== Primitives ==================================================
    virtual void DrawArrays(PrimitiveType mode, int first, int count) = 0;
    virtual void DrawElements(PrimitiveType mode, int count,
                              IndexType indexType, const void* indices) = 0;

    //=== State =======================================================
    virtual void SetLineWidth(float width) = 0;
    virtual void SetLineSmoothing(bool enable) = 0;
    virtual void SetScissor(int x, int y, int width, int height, bool enable) = 0;

    //=== Vertex attributes ===========================================
    virtual void SetConstantVertexAttrib4f(unsigned int index,
                                           float v0, float v1, float v2, float v3) = 0;
    virtual void EnableVertexAttribArray(unsigned int index, bool enable) = 0;

    //=== Framebuffer =================================================
    virtual void BindDefaultFramebuffer() = 0;
    virtual void CopyFramebufferToTexture(int srcX0, int srcY0, int srcX1, int srcY1,
                                          int dstX0, int dstY0, int dstWidth, int dstHeight) = 0;

    //=== Shader language version =====================================
    struct GlslVersion { int major{}; int minor{}; };
    virtual auto GetShaderLanguageVersion() const -> GlslVersion = 0;

    //=== Resource factory ============================================
    virtual auto CreateTexture(std::string name, int width, int height,
                               bool isUserTexture) -> std::shared_ptr<Texture> = 0;
    virtual auto CreateTexture(std::string name, int width, int height, int depth,
                               TextureFormat internalFormat) -> std::shared_ptr<Texture> = 0;
    virtual auto CreateFramebuffer(int count) -> std::shared_ptr<Framebuffer> = 0;
    virtual auto CreateShaderProgram(std::string_view vertexSrc,
                                     std::string_view fragmentSrc) -> std::shared_ptr<ShaderProgram> = 0;
    virtual auto CreateSampler(SamplerWrap wrapMode, SamplerFilter filterMode) -> std::shared_ptr<Sampler> = 0;
    virtual auto CreateMesh() -> std::shared_ptr<VertexMesh> = 0;
};

} // namespace Backend
} // namespace Renderer
} // namespace libprojectM
