#include <mbgl/gl/layer_group_gl.hpp>

#include <mbgl/gfx/drawable_tweaker.hpp>
#include <mbgl/gfx/render_pass.hpp>
#include <mbgl/gfx/renderable.hpp>
#include <mbgl/gfx/renderer_backend.hpp>
#include <mbgl/gfx/upload_pass.hpp>
#include <mbgl/gl/drawable_gl.hpp>
#include <mbgl/renderer/paint_parameters.hpp>
#include <mbgl/shaders/gl/shader_program_gl.hpp>
#include <mbgl/util/convert.hpp>

namespace mbgl {
namespace gl {

TileLayerGroupGL::TileLayerGroupGL(int32_t layerIndex_, std::size_t initialCapacity, std::string name_)
    : TileLayerGroup(layerIndex_, initialCapacity, std::move(name_)) {}

void TileLayerGroupGL::upload(gfx::UploadPass& uploadPass) {
    if (!enabled) {
        return;
    }

    visitDrawables([&](gfx::Drawable& drawable) {
        if (!drawable.getEnabled()) {
            return;
        }

        auto& drawableGL = static_cast<gl::DrawableGL&>(drawable);

#if !defined(NDEBUG)
        std::string label;
        if (const auto& tileID = drawable.getTileID()) {
            label = drawable.getName() + "/" + util::toString(*tileID);
        }
        const auto labelPtr = (label.empty() ? drawable.getName() : label).c_str();
        const auto debugGroup = uploadPass.createDebugGroup(labelPtr);
#endif

        drawableGL.upload(uploadPass);
    });
}

void TileLayerGroupGL::render(RenderOrchestrator&, PaintParameters& parameters) {
    if (!enabled) {
        return;
    }

    auto& context = static_cast<gl::Context&>(parameters.context);

    // `stencilModeFor3D` uses a different stencil mask value each time its called, so if the
    // drawables in this layer use 3D stencil mode, we need to set it up here so that all the
    // drawables end up using the same mode value.
    // 2D and 3D features in the same layer group is not supported.
    bool features3d = false;
    bool stencil3d = false;
    gfx::StencilMode stencilMode3d;

    if (getDrawableCount()) {
#if !defined(NDEBUG)
        const auto label_clip = getName() + (getName().empty() ? "" : "-") + "tile-clip-masks";
        const auto debugGroupClip = parameters.encoder->createDebugGroup(label_clip.c_str());
#endif

        // Collect the tile IDs relevant to stenciling and update the stencil buffer, if necessary.
        std::set<UnwrappedTileID> tileIDs;
        visitDrawables([&](const gfx::Drawable& drawable) {
            if (!drawable.getEnabled() || !drawable.hasRenderPass(parameters.pass)) {
                return;
            }
            if (drawable.getIs3D()) {
                features3d = true;
                if (drawable.getEnableStencil()) {
                    stencil3d = true;
                }
            }
            if (!features3d && drawable.getEnableStencil() && drawable.getTileID()) {
                tileIDs.emplace(drawable.getTileID()->toUnwrapped());
            }
        });

        // If we're doing 3D stenciling and have any features
        // to draw, set up the single-value stencil mask.
        // If we're doing 2D stenciling and have any drawables with tile IDs,
        // render each tile into the stencil buffer with a different value.
        if (features3d) {
            stencilMode3d = stencil3d ? parameters.stencilModeFor3D() : gfx::StencilMode::disabled();
        } else if (!tileIDs.empty()) {
            parameters.renderTileClippingMasks(tileIDs);
        }
    }

#if !defined(NDEBUG)
    const auto label_render = getName() + (getName().empty() ? "" : "-") + "render";
    const auto debugGroupRender = parameters.encoder->createDebugGroup(label_render.c_str());
#endif

    visitDrawables([&](gfx::Drawable& drawable) {
        if (!drawable.getEnabled() || !drawable.hasRenderPass(parameters.pass)) {
            return;
        }

#if !defined(NDEBUG)
        std::string label_tile;
        if (const auto& tileID = drawable.getTileID()) {
            label_tile = util::toString(drawable.getID().id()) + "/" + drawable.getName() + "/" +
                         util::toString(*tileID);
        }
        const auto labelPtr = (label_tile.empty() ? drawable.getName() : label_tile).c_str();
        const auto debugGroupTile = parameters.encoder->createDebugGroup(labelPtr);
#endif

        for (const auto& tweaker : drawable.getTweakers()) {
            tweaker->execute(drawable, parameters);
        }

        // For layer groups with 3D features, enable either the single-value
        // stencil mode for features with stencil enabled or disable stenciling.
        // 2D drawables will set their own stencil mode within `draw`.
        if (features3d) {
            context.setStencilMode(drawable.getEnableStencil() ? stencilMode3d : gfx::StencilMode::disabled());
        }

        drawable.draw(parameters);
    });
}

LayerGroupGL::LayerGroupGL(int32_t layerIndex_, std::size_t initialCapacity, std::string name_)
    : LayerGroup(layerIndex_, initialCapacity, std::move(name_)) {}

void LayerGroupGL::upload(gfx::UploadPass& uploadPass) {
    if (!enabled) {
        return;
    }

    visitDrawables([&](gfx::Drawable& drawable) {
        if (!drawable.getEnabled()) {
            return;
        }

        auto& drawableGL = static_cast<gl::DrawableGL&>(drawable);

#if !defined(NDEBUG)
        const auto debugGroup = uploadPass.createDebugGroup(drawable.getName().c_str());
#endif

        drawableGL.upload(uploadPass);
    });
}

void LayerGroupGL::render(RenderOrchestrator&, PaintParameters& parameters) {
    if (!enabled) {
        return;
    }

    visitDrawables([&](gfx::Drawable& drawable) {
        if (!drawable.getEnabled() || !drawable.hasRenderPass(parameters.pass)) {
            return;
        }

#if !defined(NDEBUG)
        const auto debugGroup = parameters.encoder->createDebugGroup(drawable.getName().c_str());
#endif

        for (const auto& tweaker : drawable.getTweakers()) {
            tweaker->execute(drawable, parameters);
        }

        drawable.draw(parameters);
    });
}

} // namespace gl
} // namespace mbgl
