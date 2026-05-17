#pragma once

#include "Renderer/Backend/GraphicsBackend.hpp"

#include <memory>

namespace libprojectM {
namespace Renderer {
namespace Backend {

/**
 * @brief Creates the appropriate graphics backend for the current build configuration.
 */
auto CreateGraphicsBackend() -> std::shared_ptr<GraphicsBackend>;

} // namespace Backend
} // namespace Renderer
} // namespace libprojectM
