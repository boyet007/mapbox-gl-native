#include <mbgl/renderer/painter.hpp>
#include <mbgl/renderer/paint_parameters.hpp>
#include <mbgl/renderer/circle_bucket.hpp>
#include <mbgl/renderer/render_tile.hpp>
#include <mbgl/style/layers/circle_layer.hpp>
#include <mbgl/style/layers/circle_layer_impl.hpp>
#include <mbgl/shader/shaders.hpp>
#include <mbgl/shader/circle_uniforms.hpp>
#include <mbgl/gl/context.hpp>

namespace mbgl {

using namespace style;

void Painter::renderCircle(PaintParameters& parameters,
                           CircleBucket& bucket,
                           const CircleLayer& layer,
                           const RenderTile& tile) {
    if (pass == RenderPass::Opaque) {
        return;
    }

    const CirclePaintProperties& properties = layer.impl->paint;
    const bool scaleWithMap = properties.circlePitchScale.value == CirclePitchScaleType::Map;

    context.draw({
        depthModeForSublayer(0, gl::DepthMode::ReadOnly),
        frame.mapMode == MapMode::Still
            ? stencilModeForClipping(tile.clip)
            : gl::StencilMode::disabled(),
        colorModeForRenderPass(),
        parameters.shaders.circle,
        CircleUniforms::Values {
            uniforms::u_matrix::Value{ tile.translatedMatrix(properties.circleTranslate.value,
                                       properties.circleTranslateAnchor.value,
                                       state) },
            uniforms::u_opacity::Value{ properties.circleOpacity.value },
            uniforms::u_color::Value{ properties.circleColor.value },
            uniforms::u_radius::Value{ properties.circleRadius.value },
            uniforms::u_blur::Value{ properties.circleBlur.value },
            uniforms::u_scale_with_map::Value{ scaleWithMap },
            uniforms::u_extrude_scale::Value{ scaleWithMap
                ? std::array<float, 2> {{
                    pixelsToGLUnits[0] * state.getAltitude(),
                    pixelsToGLUnits[1] * state.getAltitude()
                  }}
                : pixelsToGLUnits },
            uniforms::u_devicepixelratio::Value{ frame.pixelRatio },
        },
        gl::Segmented<gl::Triangles>(
            *bucket.vertexBuffer,
            *bucket.indexBuffer,
            bucket.segments
        )
    });
}

} // namespace mbgl
