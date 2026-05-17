#pragma once

#include "Renderer/Backend/GraphicsBackend.hpp"
#include "Renderer/Backend/Resources.hpp"

#include <memory>
#include <string>
#include <vector>

#ifdef PROJECTM_ENABLE_VULKAN

#include <vulkan/vulkan.h>

// Forward-declare VMA types (defined via vk_mem_alloc.h in the implementation file)
struct VmaAllocator_T;
using VmaAllocator = VmaAllocator_T*;
struct VmaAllocation_T;
using VmaAllocation = VmaAllocation_T*;

namespace libprojectM {
namespace Renderer {
namespace Backend {
namespace Vulkan {

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

    //=== Vulkan-specific ===========================================
    auto GetInstance() const -> VkInstance { return m_instance; }
    auto GetDevice() const -> VkDevice { return m_device; }
    auto GetPhysicalDevice() const -> VkPhysicalDevice { return m_physicalDevice; }
    auto GetAllocator() const -> VmaAllocator { return m_allocator; }
    auto GetSwapChainFormat() const -> VkFormat { return m_swapChainFormat; }
    auto IsHDREnabled() const -> bool { return m_hdrEnabled; }

    void SetSurface(VkSurfaceKHR surface, int width, int height);
    void BeginFrame();
    void EndFrame();

private:
    void InitVulkan();
    void CleanupVulkan();
    void CreateInstance();
    void PickPhysicalDevice();
    void CreateDevice();
    void InitAllocator();
    void InitCommandPool();
    void CreateSwapChain();
    void DestroySwapChain();

    static auto CompileGLSLtoSPIRV(VkShaderStageFlagBits stage,
                                   const char* source,
                                   VkShaderModule& outModule,
                                   VkDevice device) -> bool;

    VkInstance m_instance{VK_NULL_HANDLE};
    VkPhysicalDevice m_physicalDevice{VK_NULL_HANDLE};
    VkDevice m_device{VK_NULL_HANDLE};
    VmaAllocator m_allocator{VK_NULL_HANDLE};
    uint32_t m_graphicsQueueFamily{0};
    VkQueue m_graphicsQueue{VK_NULL_HANDLE};

    VkCommandPool m_commandPool{VK_NULL_HANDLE};
    VkCommandBuffer m_commandBuffer{VK_NULL_HANDLE};
    VkFence m_frameFence{VK_NULL_HANDLE};

    VkSurfaceKHR m_surface{VK_NULL_HANDLE};
    VkSwapchainKHR m_swapChain{VK_NULL_HANDLE};
    std::vector<VkImage> m_swapChainImages;
    std::vector<VkImageView> m_swapChainImageViews;
    VkFormat m_swapChainFormat{VK_FORMAT_UNDEFINED};
    VkColorSpaceKHR m_swapChainColorSpace{VK_COLOR_SPACE_SRGB_NONLINEAR_KHR};
    VkExtent2D m_swapChainExtent{};
    int m_surfaceWidth{0};
    int m_surfaceHeight{0};
    bool m_hdrEnabled{false};
    bool m_hdrMetadataSupported{false};

    PFN_vkSetHdrMetadataEXT m_vkSetHdrMetadataEXT{nullptr};

    // Pipeline state tracking
    VkViewport m_viewport{};
    VkRect2D m_scissor{};
    float m_clearColor[4]{0, 0, 0, 1};
    float m_constantAttrib[4]{1, 1, 1, 1};

    //=== Resource adapter classes ===================================

    class VkTextureAdapter : public Backend::Texture
    {
    public:
        VkTextureAdapter(std::string name, VkDevice device, VmaAllocator allocator,
                         VkImage image, VmaAllocation allocation, VkImageView view, VkFormat format,
                         int width, int height, int depth, bool isUserTexture);
        ~VkTextureAdapter() override;

        auto Name() const -> const std::string& override { return m_name; }
        auto Width() const -> int override { return m_width; }
        auto Height() const -> int override { return m_height; }
        auto Depth() const -> int override { return m_depth; }
        auto IsUserTexture() const -> bool override { return m_isUserTexture; }
        auto Empty() const -> bool override { return m_image == VK_NULL_HANDLE; }
        void Bind(int slot, const std::shared_ptr<Backend::Sampler>& sampler) override;
        void Unbind(int slot) override;
        void Update(const void* data) override;

        auto Image() const -> VkImage { return m_image; }
        auto View() const -> VkImageView { return m_view; }

    private:
        std::string m_name;
        VkDevice m_device;
        VmaAllocator m_allocator;
        VkImage m_image;
        VmaAllocation m_allocation;
        VkImageView m_view;
        VkFormat m_format;
        int m_width, m_height, m_depth;
        bool m_isUserTexture;
    };

    class VkFramebufferAdapter : public Backend::Framebuffer
    {
    public:
        VkFramebufferAdapter(int count, VkDevice device, VmaAllocator allocator,
                             VkGraphicsBackend* backend);
        ~VkFramebufferAdapter() override;

        auto Count() const -> int override { return m_count; }
        void Bind(int index) override;
        void BindRead(int index) override;
        void BindDraw(int index) override;
        void Unbind() override;
        auto SetSize(int width, int height) -> bool override;
        auto Width() const -> int override { return m_width; }
        auto Height() const -> int override { return m_height; }
        auto GetColorAttachmentTexture(int idx, int attIdx) const -> std::shared_ptr<Backend::Texture> override;
        void CreateColorAttachment(int idx, int attIdx) override;
        void RemoveColorAttachment(int idx, int attIdx) override;
        void SetAttachment(int idx, int attIdx, const std::shared_ptr<TextureAttachment>& attachment) override;
        void MaskDrawBuffer(int bufIdx, bool masked) override;

    private:
        void CreateVkFramebuffer(int index);
        void DestroyVkFramebuffer(int index);

        VkDevice m_device;
        VmaAllocator m_allocator;
        VkGraphicsBackend* m_backend;
        int m_count;
        int m_width{0};
        int m_height{0};

        struct BufferAttachments
        {
            std::vector<std::shared_ptr<Backend::Texture>> colorTextures;
            std::vector<bool> colorMasks;
            VkRenderPass renderPass{VK_NULL_HANDLE};
            VkFramebuffer framebuffer{VK_NULL_HANDLE};
        };
        std::vector<BufferAttachments> m_buffers;
    };

    class VkShaderProgramAdapter : public Backend::ShaderProgram
    {
    public:
        VkShaderProgramAdapter(VkPipeline pipeline, VkPipelineLayout pipelineLayout,
                               std::vector<VkShaderModule> modules, VkDevice device);
        ~VkShaderProgramAdapter() override;

        void Bind() override;
        void Unbind() override;
        auto Validate(std::string& msg) const -> bool override;
        void SetUniformFloat(const char* u, float v) override;
        void SetUniformInt(const char* u, int v) override;
        void SetUniformFloat2(const char* u, const glm::vec2& v) override;
        void SetUniformInt2(const char* u, const glm::ivec2& v) override;
        void SetUniformFloat3(const char* u, const glm::vec3& v) override;
        void SetUniformInt3(const char* u, const glm::ivec3& v) override;
        void SetUniformFloat4(const char* u, const glm::vec4& v) override;
        void SetUniformInt4(const char* u, const glm::ivec4& v) override;
        void SetUniformMat3x4(const char* u, const glm::mat3x4& v) override;
        void SetUniformMat4x4(const char* u, const glm::mat4x4& v) override;

        auto Pipeline() const -> VkPipeline { return m_pipeline; }
        auto PipelineLayout() const -> VkPipelineLayout { return m_pipelineLayout; }

    private:
        VkPipeline m_pipeline;
        VkPipelineLayout m_pipelineLayout;
        std::vector<VkShaderModule> m_modules;
        VkDevice m_device;
    };

    class VkSamplerAdapter : public Backend::Sampler
    {
    public:
        VkSamplerAdapter(VkSampler sampler, VkDevice device,
                         SamplerWrap wrapMode, SamplerFilter filterMode);
        ~VkSamplerAdapter() override;

        void Bind(unsigned int unit) override;
        void Unbind(unsigned int unit) override;
        auto WrapMode() const -> SamplerWrap override { return m_wrapMode; }
        void WrapMode(SamplerWrap mode) override;
        auto FilterMode() const -> SamplerFilter override { return m_filterMode; }
        void FilterMode(SamplerFilter mode) override;

        auto Handle() const -> VkSampler { return m_sampler; }

    private:
        VkSampler m_sampler;
        VkDevice m_device;
        SamplerWrap m_wrapMode;
        SamplerFilter m_filterMode;
    };

    class VkVertexMeshAdapter : public Backend::VertexMesh
    {
    public:
        VkVertexMeshAdapter(VkDevice device, VmaAllocator allocator,
                            VkGraphicsBackend* backend);
        ~VkVertexMeshAdapter() override;

        void Bind() override;
        void Unbind() override;
        void Draw() override;
        void Update() override;

    private:
        VkDevice m_device;
        VmaAllocator m_allocator;
        VkGraphicsBackend* m_backend;
        VkBuffer m_vertexBuffer{VK_NULL_HANDLE};
        VmaAllocation m_vertexAllocation{VK_NULL_HANDLE};
        VkBuffer m_indexBuffer{VK_NULL_HANDLE};
        VmaAllocation m_indexAllocation{VK_NULL_HANDLE};
    };
};

} // namespace Vulkan
} // namespace Backend
} // namespace Renderer
} // namespace libprojectM

#endif // PROJECTM_ENABLE_VULKAN
