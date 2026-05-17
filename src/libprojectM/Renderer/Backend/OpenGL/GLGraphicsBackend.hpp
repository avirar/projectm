#pragma once

#include "Renderer/Backend/GraphicsBackend.hpp"
#include "Renderer/Backend/Resources.hpp"
#include "Renderer/BlendMode.hpp"
#include "Renderer/Framebuffer.hpp"
#include "Renderer/Mesh.hpp"
#include "Renderer/Sampler.hpp"
#include "Renderer/Shader.hpp"
#include "Renderer/Texture.hpp"
#include "Renderer/TextureAttachment.hpp"

#include <memory>
#include <string>

namespace libprojectM {
namespace Renderer {
namespace Backend {
namespace OpenGL {

/**
 * @brief OpenGL implementation of the GraphicsBackend interface.
 *
 * Wraps the existing Renderer::* classes to provide the backend abstraction.
 */
class GLGraphicsBackend : public GraphicsBackend
{
public:
    GLGraphicsBackend();
    ~GLGraphicsBackend() override = default;

    auto Name() const -> std::string_view override { return "OpenGL"; }

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
    void BindDrawFramebufferRaw(uint32_t fboId) override;
    void BindReadFramebufferRaw(uint32_t fboId) override;
    auto GetDrawFramebufferBindingRaw() -> uint32_t override;
    auto GetReadFramebufferBindingRaw() -> uint32_t override;
    void CopyFramebufferToTexture(int srcX0, int srcY0, int srcX1, int srcY1,
                                  int dstX0, int dstY0, int dstWidth, int dstHeight) override;
    void EnsureDefaultDrawBuffers() override;

    void SetBoundTextureWrap(SamplerWrap wrapS, SamplerWrap wrapT) override;

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

private:
    static auto ToGLPrimitiveType(PrimitiveType type) -> GLenum;
    static auto ToGLIndexType(IndexType type) -> GLenum;
    static auto ToGLBlendFactor(BlendFactor factor) -> GLenum;
    static auto ToGLSamplerWrap(SamplerWrap wrap) -> GLint;
    static auto ToGLSamplerFilter(SamplerFilter filter) -> GLint;
    static auto ToGLInternalFormat(TextureFormat format) -> GLint;
    static auto ToGLFormat(TextureFormat format) -> GLenum;
    static auto ToGLType(TextureFormat format) -> GLenum;

    class GLTextureAdapter : public Backend::Texture
    {
    public:
        explicit GLTextureAdapter(std::shared_ptr<Renderer::Texture> tex)
            : m_impl(std::move(tex)) {}
        auto Name() const -> const std::string& override { return m_impl->Name(); }
        auto Width() const -> int override { return m_impl->Width(); }
        auto Height() const -> int override { return m_impl->Height(); }
        auto Depth() const -> int override { return m_impl->Depth(); }
        auto IsUserTexture() const -> bool override { return m_impl->IsUserTexture(); }
        auto Empty() const -> bool override { return m_impl->Empty(); }
        void Bind(int slot, const std::shared_ptr<Backend::Sampler>& sampler) override;
        void Unbind(int slot) override { m_impl->Unbind(slot); }
        void Update(const void* data) override { m_impl->Update(data); }
        auto Impl() const -> std::shared_ptr<Renderer::Texture> { return m_impl; }
    private:
        std::shared_ptr<Renderer::Texture> m_impl;
    };

    class GLFramebufferAdapter : public Backend::Framebuffer
    {
    public:
        explicit GLFramebufferAdapter(int count) : m_impl(std::make_shared<Renderer::Framebuffer>(count)) {}
        explicit GLFramebufferAdapter(std::shared_ptr<Renderer::Framebuffer> fb) : m_impl(std::move(fb)) {}
        auto Count() const -> int override { return m_impl->Count(); }
        void Bind(int index) override { m_impl->Bind(index); }
        void BindRead(int index) override { m_impl->BindRead(index); }
        void BindDraw(int index) override { m_impl->BindDraw(index); }
        void Unbind() override { Renderer::Framebuffer::Unbind(); }
        auto SetSize(int width, int height) -> bool override { return m_impl->SetSize(width, height); }
        auto Width() const -> int override { return m_impl->Width(); }
        auto Height() const -> int override { return m_impl->Height(); }
        auto GetColorAttachmentTexture(int idx, int attIdx) const -> std::shared_ptr<Backend::Texture> override;
        void CreateColorAttachment(int idx, int attIdx) override { m_impl->CreateColorAttachment(idx, attIdx); }
        void RemoveColorAttachment(int idx, int attIdx) override { m_impl->RemoveColorAttachment(idx, attIdx); }
        void SetAttachment(int idx, int attIdx, const std::shared_ptr<TextureAttachment>& attachment) override;
        void MaskDrawBuffer(int bufIdx, bool masked) override { m_impl->MaskDrawBuffer(bufIdx, masked); }
        auto Impl() const -> std::shared_ptr<Renderer::Framebuffer> { return m_impl; }
    private:
        std::shared_ptr<Renderer::Framebuffer> m_impl;
    };

    class GLShaderProgramAdapter : public Backend::ShaderProgram
    {
    public:
        explicit GLShaderProgramAdapter(std::shared_ptr<Renderer::Shader> shader)
            : m_impl(std::move(shader)) {}
        void Bind() override { m_impl->Bind(); }
        void Unbind() override { Renderer::Shader::Unbind(); }
        auto Validate(std::string& msg) const -> bool override { return m_impl->Validate(msg); }
        void SetUniformFloat(const char* u, float v) override { m_impl->SetUniformFloat(u, v); }
        void SetUniformInt(const char* u, int v) override { m_impl->SetUniformInt(u, v); }
        void SetUniformFloat2(const char* u, const glm::vec2& v) override { m_impl->SetUniformFloat2(u, v); }
        void SetUniformInt2(const char* u, const glm::ivec2& v) override { m_impl->SetUniformInt2(u, v); }
        void SetUniformFloat3(const char* u, const glm::vec3& v) override { m_impl->SetUniformFloat3(u, v); }
        void SetUniformInt3(const char* u, const glm::ivec3& v) override { m_impl->SetUniformInt3(u, v); }
        void SetUniformFloat4(const char* u, const glm::vec4& v) override { m_impl->SetUniformFloat4(u, v); }
        void SetUniformInt4(const char* u, const glm::ivec4& v) override { m_impl->SetUniformInt4(u, v); }
        void SetUniformMat3x4(const char* u, const glm::mat3x4& v) override { m_impl->SetUniformMat3x4(u, v); }
        void SetUniformMat4x4(const char* u, const glm::mat4x4& v) override { m_impl->SetUniformMat4x4(u, v); }
        auto Impl() const -> std::shared_ptr<Renderer::Shader> { return m_impl; }
    private:
        std::shared_ptr<Renderer::Shader> m_impl;
    };

    class GLSamplerAdapter : public Backend::Sampler
    {
    public:
        explicit GLSamplerAdapter(std::shared_ptr<Renderer::Sampler> sampler)
            : m_impl(std::move(sampler)) {}
        void Bind(unsigned int unit) override { m_impl->Bind(unit); }
        void Unbind(unsigned int unit) override { Renderer::Sampler::Unbind(unit); }
        auto WrapMode() const -> SamplerWrap override;
        void WrapMode(SamplerWrap mode) override;
        auto FilterMode() const -> SamplerFilter override;
        void FilterMode(SamplerFilter mode) override;
        auto Impl() const -> std::shared_ptr<Renderer::Sampler> { return m_impl; }
    private:
        std::shared_ptr<Renderer::Sampler> m_impl;
    };

    class GLVertexMeshAdapter : public Backend::VertexMesh
    {
    public:
        explicit GLVertexMeshAdapter(std::shared_ptr<Renderer::Mesh> mesh)
            : m_impl(std::move(mesh)) {}
        void Bind() override { m_impl->Bind(); }
        void Unbind() override { Renderer::Mesh::Unbind(); }
        void Draw() override { m_impl->Draw(); }
        void Update() override { m_impl->Update(); }
        auto Impl() const -> std::shared_ptr<Renderer::Mesh> { return m_impl; }
    private:
        std::shared_ptr<Renderer::Mesh> m_impl;
    };
};

} // namespace OpenGL
} // namespace Backend
} // namespace Renderer
} // namespace libprojectM
