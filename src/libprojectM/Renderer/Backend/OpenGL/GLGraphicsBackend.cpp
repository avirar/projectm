#include "GLGraphicsBackend.hpp"

#include <Renderer/OpenGL.h>

#include <stdexcept>

namespace libprojectM {
namespace Renderer {
namespace Backend {
namespace OpenGL {

GLGraphicsBackend::GLGraphicsBackend() = default;

void GLGraphicsBackend::SetViewport(int x, int y, int width, int height)
{
    glViewport(x, y, width, height);
}

void GLGraphicsBackend::ClearColor(float r, float g, float b, float a)
{
    glClearColor(r, g, b, a);
}

void GLGraphicsBackend::Clear()
{
    glClear(GL_COLOR_BUFFER_BIT);
}

void GLGraphicsBackend::SetBlendMode(BlendFactor srcRgb, BlendFactor dstRgb,
                                     BlendFactor srcAlpha, BlendFactor dstAlpha,
                                     bool enable)
{
    if (enable)
    {
        glEnable(GL_BLEND);
        glBlendFuncSeparate(ToGLBlendFactor(srcRgb), ToGLBlendFactor(dstRgb),
                            ToGLBlendFactor(srcAlpha), ToGLBlendFactor(dstAlpha));
    }
    else
    {
        glDisable(GL_BLEND);
    }
}

void GLGraphicsBackend::DrawArrays(PrimitiveType mode, int first, int count)
{
    glDrawArrays(ToGLPrimitiveType(mode), first, count);
}

void GLGraphicsBackend::DrawElements(PrimitiveType mode, int count,
                                     IndexType indexType, const void* indices)
{
    glDrawElements(ToGLPrimitiveType(mode), count, ToGLIndexType(indexType), indices);
}

void GLGraphicsBackend::SetLineWidth(float width)
{
    glLineWidth(width);
}

void GLGraphicsBackend::SetLineSmoothing(bool enable)
{
    if (enable)
        glEnable(GL_LINE_SMOOTH);
    else
        glDisable(GL_LINE_SMOOTH);
}

void GLGraphicsBackend::SetScissor(int x, int y, int width, int height, bool enable)
{
    if (enable)
    {
        glEnable(GL_SCISSOR_TEST);
        glScissor(x, y, width, height);
    }
    else
    {
        glDisable(GL_SCISSOR_TEST);
    }
}

void GLGraphicsBackend::SetConstantVertexAttrib4f(unsigned int index,
                                                  float v0, float v1, float v2, float v3)
{
    glVertexAttrib4f(index, v0, v1, v2, v3);
}

void GLGraphicsBackend::EnableVertexAttribArray(unsigned int index, bool enable)
{
    if (enable)
        glEnableVertexAttribArray(index);
    else
        glDisableVertexAttribArray(index);
}

void GLGraphicsBackend::BindDefaultFramebuffer()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void GLGraphicsBackend::CopyFramebufferToTexture(int srcX0, int srcY0, int srcX1, int srcY1,
                                                 int dstX0, int dstY0, int dstWidth, int dstHeight)
{
    glCopyTexSubImage2D(GL_TEXTURE_2D, 0, dstX0, dstY0, srcX0, srcY0,
                        srcX1 - srcX0, srcY1 - srcY0);
}

auto GLGraphicsBackend::GetShaderLanguageVersion() const -> GlslVersion
{
    auto version = Renderer::Shader::GetShaderLanguageVersion();
    return {version.major, version.minor};
}

auto GLGraphicsBackend::CreateTexture(std::string name, int width, int height,
                                      bool isUserTexture) -> std::shared_ptr<Texture>
{
    auto tex = std::make_shared<Renderer::Texture>(std::move(name), width, height, isUserTexture);
    return std::make_shared<GLTextureAdapter>(std::move(tex));
}

auto GLGraphicsBackend::CreateTexture(std::string name, int width, int height, int depth,
                                      TextureFormat internalFormat) -> std::shared_ptr<Texture>
{
    auto target = (depth > 1) ? GL_TEXTURE_3D : GL_TEXTURE_2D;
    auto tex = std::make_shared<Renderer::Texture>(
        std::move(name), target, width, height, depth,
        ToGLInternalFormat(internalFormat),
        ToGLFormat(internalFormat),
        ToGLType(internalFormat),
        false);
    return std::make_shared<GLTextureAdapter>(std::move(tex));
}

auto GLGraphicsBackend::CreateFramebuffer(int count) -> std::shared_ptr<Framebuffer>
{
    return std::make_shared<GLFramebufferAdapter>(count);
}

auto GLGraphicsBackend::CreateShaderProgram(std::string_view vertexSrc,
                                            std::string_view fragmentSrc) -> std::shared_ptr<ShaderProgram>
{
    auto shader = std::make_shared<Renderer::Shader>();
    shader->CompileProgram(std::string(vertexSrc), std::string(fragmentSrc));
    return std::make_shared<GLShaderProgramAdapter>(std::move(shader));
}

auto GLGraphicsBackend::CreateSampler(SamplerWrap wrapMode, SamplerFilter filterMode) -> std::shared_ptr<Sampler>
{
    auto sampler = std::make_shared<Renderer::Sampler>(ToGLSamplerWrap(wrapMode), ToGLSamplerFilter(filterMode));
    return std::make_shared<GLSamplerAdapter>(std::move(sampler));
}

auto GLGraphicsBackend::CreateMesh() -> std::shared_ptr<VertexMesh>
{
    return std::make_shared<GLVertexMeshAdapter>(std::make_shared<Renderer::Mesh>());
}

//=== GLTextureAdapter =======================================
void GLGraphicsBackend::GLTextureAdapter::Bind(int slot, const std::shared_ptr<Backend::Sampler>& sampler)
{
    if (sampler)
    {
        auto* glSampler = dynamic_cast<GLSamplerAdapter*>(sampler.get());
        m_impl->Bind(slot, glSampler ? glSampler->Impl() : nullptr);
    }
    else
    {
        m_impl->Bind(slot);
    }
}

//=== GLFramebufferAdapter ===================================
auto GLGraphicsBackend::GLFramebufferAdapter::GetColorAttachmentTexture(int idx, int attIdx) const -> std::shared_ptr<Backend::Texture>
{
    auto tex = m_impl->GetColorAttachmentTexture(idx, attIdx);
    if (!tex)
        return nullptr;
    return std::make_shared<GLTextureAdapter>(std::move(tex));
}

void GLGraphicsBackend::GLFramebufferAdapter::SetAttachment(int idx, int attIdx, const std::shared_ptr<Backend::TextureAttachment>& attachment)
{
    if (!attachment)
    {
        m_impl->RemoveColorAttachment(idx, attIdx);
        return;
    }
    // TextureAttachment is Renderer::TextureAttachment - cast through
    auto* glAttach = dynamic_cast<Renderer::TextureAttachment*>(attachment.get());
    if (glAttach)
        m_impl->SetAttachment(idx, attIdx, std::shared_ptr<Renderer::TextureAttachment>(attachment, glAttach));
}

//=== GLSamplerAdapter =======================================
auto GLGraphicsBackend::GLSamplerAdapter::WrapMode() const -> SamplerWrap
{
    switch (m_impl->WrapMode())
    {
        case GL_REPEAT: return SamplerWrap::Repeat;
        case GL_MIRRORED_REPEAT: return SamplerWrap::MirroredRepeat;
        case GL_CLAMP_TO_EDGE: return SamplerWrap::ClampToEdge;
        case GL_CLAMP_TO_BORDER: return SamplerWrap::ClampToBorder;
        default: return SamplerWrap::ClampToEdge;
    }
}

void GLGraphicsBackend::GLSamplerAdapter::WrapMode(SamplerWrap mode)
{
    m_impl->WrapMode(ToGLSamplerWrap(mode));
}

auto GLGraphicsBackend::GLSamplerAdapter::FilterMode() const -> SamplerFilter
{
    switch (m_impl->FilterMode())
    {
        case GL_NEAREST: return SamplerFilter::Nearest;
        case GL_LINEAR: return SamplerFilter::Linear;
        default: return SamplerFilter::Linear;
    }
}

void GLGraphicsBackend::GLSamplerAdapter::FilterMode(SamplerFilter mode)
{
    m_impl->FilterMode(ToGLSamplerFilter(mode));
}

//=== Type conversions =======================================

auto GLGraphicsBackend::ToGLPrimitiveType(PrimitiveType type) -> GLenum
{
    switch (type)
    {
        case PrimitiveType::Points: return GL_POINTS;
        case PrimitiveType::Lines: return GL_LINES;
        case PrimitiveType::LineStrip: return GL_LINE_STRIP;
        case PrimitiveType::LineLoop: return GL_LINE_LOOP;
        case PrimitiveType::Triangles: return GL_TRIANGLES;
        case PrimitiveType::TriangleStrip: return GL_TRIANGLE_STRIP;
        case PrimitiveType::TriangleFan: return GL_TRIANGLE_FAN;
    }
    return GL_TRIANGLES;
}

auto GLGraphicsBackend::ToGLIndexType(IndexType type) -> GLenum
{
    switch (type)
    {
        case IndexType::UnsignedByte: return GL_UNSIGNED_BYTE;
        case IndexType::UnsignedShort: return GL_UNSIGNED_SHORT;
        case IndexType::UnsignedInt: return GL_UNSIGNED_INT;
    }
    return GL_UNSIGNED_INT;
}

auto GLGraphicsBackend::ToGLBlendFactor(BlendFactor factor) -> GLenum
{
    switch (factor)
    {
        case BlendFactor::Zero: return GL_ZERO;
        case BlendFactor::One: return GL_ONE;
        case BlendFactor::SrcColor: return GL_SRC_COLOR;
        case BlendFactor::OneMinusSrcColor: return GL_ONE_MINUS_SRC_COLOR;
        case BlendFactor::DstColor: return GL_DST_COLOR;
        case BlendFactor::OneMinusDstColor: return GL_ONE_MINUS_DST_COLOR;
        case BlendFactor::SrcAlpha: return GL_SRC_ALPHA;
        case BlendFactor::OneMinusSrcAlpha: return GL_ONE_MINUS_SRC_ALPHA;
        case BlendFactor::DstAlpha: return GL_DST_ALPHA;
        case BlendFactor::OneMinusDstAlpha: return GL_ONE_MINUS_DST_ALPHA;
        case BlendFactor::ConstantColor: return GL_CONSTANT_COLOR;
        case BlendFactor::OneMinusConstantColor: return GL_ONE_MINUS_CONSTANT_COLOR;
        case BlendFactor::ConstantAlpha: return GL_CONSTANT_ALPHA;
        case BlendFactor::OneMinusConstantAlpha: return GL_ONE_MINUS_CONSTANT_ALPHA;
        case BlendFactor::SrcAlphaSaturate: return GL_SRC_ALPHA_SATURATE;
        default: return GL_ONE;
    }
}

auto GLGraphicsBackend::ToGLSamplerWrap(SamplerWrap wrap) -> GLint
{
    switch (wrap)
    {
        case SamplerWrap::Repeat: return GL_REPEAT;
        case SamplerWrap::MirroredRepeat: return GL_MIRRORED_REPEAT;
        case SamplerWrap::ClampToEdge: return GL_CLAMP_TO_EDGE;
        case SamplerWrap::ClampToBorder: return GL_CLAMP_TO_BORDER;
    }
    return GL_CLAMP_TO_EDGE;
}

auto GLGraphicsBackend::ToGLSamplerFilter(SamplerFilter filter) -> GLint
{
    switch (filter)
    {
        case SamplerFilter::Nearest: return GL_NEAREST;
        case SamplerFilter::Linear: return GL_LINEAR;
        case SamplerFilter::NearestMipmapNearest: return GL_NEAREST_MIPMAP_NEAREST;
        case SamplerFilter::LinearMipmapNearest: return GL_LINEAR_MIPMAP_NEAREST;
        case SamplerFilter::NearestMipmapLinear: return GL_NEAREST_MIPMAP_LINEAR;
        case SamplerFilter::LinearMipmapLinear: return GL_LINEAR_MIPMAP_LINEAR;
    }
    return GL_LINEAR;
}

auto GLGraphicsBackend::ToGLInternalFormat(TextureFormat format) -> GLint
{
    switch (format)
    {
        case TextureFormat::R8: return GL_R8;
        case TextureFormat::RG8: return GL_RG8;
        case TextureFormat::RGB8: return GL_RGB8;
        case TextureFormat::RGBA8: return GL_RGBA8;
        case TextureFormat::R16F: return GL_R16F;
        case TextureFormat::RG16F: return GL_RG16F;
        case TextureFormat::RGB16F: return GL_RGB16F;
        case TextureFormat::RGBA16F: return GL_RGBA16F;
        case TextureFormat::R32F: return GL_R32F;
        case TextureFormat::RG32F: return GL_RG32F;
        case TextureFormat::RGB32F: return GL_RGB32F;
        case TextureFormat::RGBA32F: return GL_RGBA32F;
        case TextureFormat::R10G10B10A2: return GL_RGB10_A2;
        case TextureFormat::SRGB8_ALPHA8: return GL_SRGB8_ALPHA8;
    }
    return GL_RGBA8;
}

auto GLGraphicsBackend::ToGLFormat(TextureFormat format) -> GLenum
{
    switch (format)
    {
        case TextureFormat::R8:
        case TextureFormat::R16F:
        case TextureFormat::R32F:
            return GL_RED;
        case TextureFormat::RG8:
        case TextureFormat::RG16F:
        case TextureFormat::RG32F:
            return GL_RG;
        case TextureFormat::RGB8:
        case TextureFormat::RGB16F:
        case TextureFormat::RGB32F:
            return GL_RGB;
        case TextureFormat::RGBA8:
        case TextureFormat::RGBA16F:
        case TextureFormat::RGBA32F:
        case TextureFormat::SRGB8_ALPHA8:
        case TextureFormat::R10G10B10A2:
            return GL_RGBA;
    }
    return GL_RGBA;
}

auto GLGraphicsBackend::ToGLType(TextureFormat format) -> GLenum
{
    switch (format)
    {
        case TextureFormat::R8:
        case TextureFormat::RG8:
        case TextureFormat::RGB8:
        case TextureFormat::RGBA8:
        case TextureFormat::SRGB8_ALPHA8:
            return GL_UNSIGNED_BYTE;
        case TextureFormat::R10G10B10A2:
            return GL_UNSIGNED_INT_2_10_10_10_REV;
        case TextureFormat::R16F:
        case TextureFormat::RG16F:
        case TextureFormat::RGB16F:
        case TextureFormat::RGBA16F:
            return GL_HALF_FLOAT;
        case TextureFormat::R32F:
        case TextureFormat::RG32F:
        case TextureFormat::RGB32F:
        case TextureFormat::RGBA32F:
            return GL_FLOAT;
    }
    return GL_UNSIGNED_BYTE;
}

} // namespace OpenGL
} // namespace Backend
} // namespace Renderer
} // namespace libprojectM
