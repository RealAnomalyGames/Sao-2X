
#pragma once

namespace sao {

enum class BlendFactor {
    Zero,
    One,
    SourceColor,
    OneMinusSourceColor,
    DestinationColor,
    OneMinusDestinationColor,
    SourceAlpha,
    OneMinusSourceAlpha,
    DestinationAlpha,
    OneMinusDestinationAlpha
};

enum class BlendOperation {
    Add,
    Subtract,
    ReverseSubtract,
    Minimum,
    Maximum
};

struct BlendState {
    bool enabled = false;

    BlendFactor source_color = BlendFactor::SourceAlpha;
    BlendFactor destination_color = BlendFactor::OneMinusSourceAlpha;

    BlendFactor source_alpha = BlendFactor::One;
    BlendFactor destination_alpha = BlendFactor::OneMinusSourceAlpha;

    BlendOperation color_operation = BlendOperation::Add;
    BlendOperation alpha_operation = BlendOperation::Add;
};

enum class DepthFunction {
    Never,
    Less,
    LessEqual,
    Equal,
    GreaterEqual,
    Greater,
    NotEqual,
    Always
};

enum class CullMode {
    None,
    Front,
    Back,
    FrontAndBack
};

enum class FrontFace {
    Clockwise,
    CounterClockwise
};

struct RenderState {
    bool depth_test_enabled = true;
    bool depth_write_enabled = true;

    DepthFunction depth_function = DepthFunction::Less;

    bool face_culling_enabled = false;
    CullMode cull_mode = CullMode::Back;
    FrontFace front_face = FrontFace::CounterClockwise;

    BlendState blending;

    bool wireframe = false;

    bool operator==(const RenderState& other) const noexcept {
        return depth_test_enabled == other.depth_test_enabled &&
               depth_write_enabled == other.depth_write_enabled &&
               depth_function == other.depth_function &&
               face_culling_enabled == other.face_culling_enabled &&
               cull_mode == other.cull_mode &&
               front_face == other.front_face &&
               blending.enabled == other.blending.enabled &&
               blending.source_color == other.blending.source_color &&
               blending.destination_color == other.blending.destination_color &&
               blending.source_alpha == other.blending.source_alpha &&
               blending.destination_alpha == other.blending.destination_alpha &&
               blending.color_operation == other.blending.color_operation &&
               blending.alpha_operation == other.blending.alpha_operation &&
               wireframe == other.wireframe;
    }
};

} // namespace sao
