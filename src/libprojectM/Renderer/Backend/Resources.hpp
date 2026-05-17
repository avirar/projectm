#pragma once

#include "Renderer/Backend/Types.hpp"

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat3x4.hpp>
#include <glm/mat4x4.hpp>

#include <memory>
#include <string>

namespace libprojectM {
namespace Renderer {
namespace Backend {

class Sampler;

class Texture
{
public:
    virtual ~Texture() = default;

    virtual auto Name() const -> const std::string& = 0;
    virtual auto Width() const -> int = 0;
    virtual auto Height() const -> int = 0;
    virtual auto Depth() const -> int = 0;
    virtual auto IsUserTexture() const -> bool = 0;
    virtual auto Empty() const -> bool = 0;

    virtual void Bind(int slot, const std::shared_ptr<Sampler>& sampler = nullptr) = 0;
    virtual void Unbind(int slot) = 0;
    virtual void Update(const void* data) = 0;
};


class Framebuffer
{
public:
    virtual ~Framebuffer() = default;

    virtual auto Count() const -> int = 0;
    virtual void Bind(int index) = 0;
    virtual void BindRead(int index) = 0;
    virtual void BindDraw(int index) = 0;
    virtual void Unbind() = 0;
    virtual auto SetSize(int width, int height) -> bool = 0;
    virtual auto Width() const -> int = 0;
    virtual auto Height() const -> int = 0;
    virtual auto GetColorAttachmentTexture(int framebufferIndex, int attachmentIndex) const -> std::shared_ptr<Texture> = 0;
    virtual void CreateColorAttachment(int framebufferIndex, int attachmentIndex) = 0;
    virtual void RemoveColorAttachment(int framebufferIndex, int attachmentIndex) = 0;
    virtual void SetAttachment(int framebufferIndex, int attachmentIndex, const std::shared_ptr<class TextureAttachment>& attachment) = 0;
    virtual void MaskDrawBuffer(int bufferIndex, bool masked) = 0;
};


class ShaderProgram
{
public:
    virtual ~ShaderProgram() = default;

    virtual void Bind() = 0;
    virtual void Unbind() = 0;
    virtual auto Validate(std::string& validationMessage) const -> bool = 0;

    virtual void SetUniformFloat(const char* uniform, float value) = 0;
    virtual void SetUniformInt(const char* uniform, int value) = 0;
    virtual void SetUniformFloat2(const char* uniform, const glm::vec2& values) = 0;
    virtual void SetUniformInt2(const char* uniform, const glm::ivec2& values) = 0;
    virtual void SetUniformFloat3(const char* uniform, const glm::vec3& values) = 0;
    virtual void SetUniformInt3(const char* uniform, const glm::ivec3& values) = 0;
    virtual void SetUniformFloat4(const char* uniform, const glm::vec4& values) = 0;
    virtual void SetUniformInt4(const char* uniform, const glm::ivec4& values) = 0;
    virtual void SetUniformMat3x4(const char* uniform, const glm::mat3x4& values) = 0;
    virtual void SetUniformMat4x4(const char* uniform, const glm::mat4x4& values) = 0;
};


class Sampler
{
public:
    virtual ~Sampler() = default;

    virtual void Bind(unsigned int unit) = 0;
    virtual void Unbind(unsigned int unit) = 0;
    virtual auto WrapMode() const -> SamplerWrap = 0;
    virtual void WrapMode(SamplerWrap mode) = 0;
    virtual auto FilterMode() const -> SamplerFilter = 0;
    virtual void FilterMode(SamplerFilter mode) = 0;
};


class VertexMesh
{
public:
    virtual ~VertexMesh() = default;

    virtual void Bind() = 0;
    virtual void Unbind() = 0;
    virtual void Draw() = 0;
    virtual void Update() = 0;
};


class TextureAttachment
{
public:
    virtual ~TextureAttachment() = default;

    virtual auto Texture() -> std::shared_ptr<Backend::Texture> = 0;
    virtual auto Texture() const -> std::shared_ptr<Backend::Texture> = 0;
    virtual auto Width() const -> int = 0;
    virtual auto Height() const -> int = 0;
    virtual void SetSize(int width, int height) = 0;
    virtual void Detach() = 0;
};

} // namespace Backend
} // namespace Renderer
} // namespace libprojectM
