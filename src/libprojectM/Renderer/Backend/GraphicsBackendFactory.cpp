#include "Renderer/Backend/GraphicsBackendFactory.hpp"
#include "Renderer/Backend/OpenGL/GLGraphicsBackend.hpp"

#ifdef PROJECTM_ENABLE_VULKAN
#include "Renderer/Backend/Vulkan/VkGraphicsBackend.hpp"
#endif

namespace libprojectM {
namespace Renderer {
namespace Backend {

auto CreateGraphicsBackend() -> std::shared_ptr<GraphicsBackend>
{
#ifdef PROJECTM_ENABLE_VULKAN
    return std::make_shared<Vulkan::VkGraphicsBackend>();
#else
    return std::make_shared<OpenGL::GLGraphicsBackend>();
#endif
}

} // namespace Backend
} // namespace Renderer
} // namespace libprojectM
