#pragma once

// lx
#include <lx/common/Rect.hpp>
#include <lx/common/non_constructible.hpp>
#include <lx/common/non_copyable.hpp>
#include <lx/containers/Vector.hpp>
#include <lx/gpu/Format.hpp>
#include <lx/gpu/RenderPass.hpp>
#include <lx/gpu/Shader.hpp>
#include <lx/gpu/Viewport.hpp>

// externals
#include <lxl/lxl.h>

// std
#include <array>
#include <span>

namespace lx::gpu {
struct pipeline : private lx::common::non_constructible
{
    enum class Kind : std::uint32_t
    {
        graphics,
        compute,
        raytracing
    };

    using enum Kind;
};

template<pipeline::Kind kind> class Pipeline : private lx::common::non_constructible
{
};

template<> class Pipeline<pipeline::graphics> : private lx::common::non_copyable
{
public:
    enum class DynamicState : std::uint32_t
    {

    };

    struct Primitive
    {
        enum class Fill : std::uint32_t
        {
            full = VK_POLYGON_MODE_FILL,
            line = VK_POLYGON_MODE_LINE,
            point = VK_POLYGON_MODE_POINT,
        };
        enum class Cull : std::uint32_t
        {
            front = VK_CULL_MODE_FRONT_BIT,
            back = VK_CULL_MODE_BACK_BIT
        };
        enum class Front : std::uint32_t
        {
            counter_clockwise = VK_FRONT_FACE_COUNTER_CLOCKWISE,
            clockwise = VK_FRONT_FACE_CLOCKWISE
        };
        enum class Topology : std::uint32_t
        {
            point_list = VK_PRIMITIVE_TOPOLOGY_POINT_LIST,
            line_list = VK_PRIMITIVE_TOPOLOGY_LINE_LIST,
            line_strip = VK_PRIMITIVE_TOPOLOGY_LINE_STRIP,
            triangle_list = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
            triangle_strip = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP,
            triangle_fan = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN,
        };

        Fill fill;
        Cull cull;
        Front front_face;
        Topology topology;
    };
    struct Depth
    {
        enum class Compare : std::uint32_t
        {
            never = VK_COMPARE_OP_NEVER,
            less = VK_COMPARE_OP_LESS,
            equal = VK_COMPARE_OP_EQUAL,
            less_or_equal = VK_COMPARE_OP_LESS_OR_EQUAL,
            greater = VK_COMPARE_OP_GREATER,
            not_equal = VK_COMPARE_OP_NOT_EQUAL,
            greater_or_equal = VK_COMPARE_OP_GREATER_OR_EQUAL,
            always = VK_COMPARE_OP_ALWAYS,
        };

        bool test = false;
        bool write = false;

        float bias_constant_factor;
        float bias_slope_factor;

        Compare compare;
    };
    struct Stencil
    {
        struct State
        {
            enum class Compare : std::uint32_t
            {
                never = VK_COMPARE_OP_NEVER,
                less = VK_COMPARE_OP_LESS,
                equal = VK_COMPARE_OP_EQUAL,
                less_or_equal = VK_COMPARE_OP_LESS_OR_EQUAL,
                greater = VK_COMPARE_OP_GREATER,
                not_equal = VK_COMPARE_OP_NOT_EQUAL,
                greater_or_equal = VK_COMPARE_OP_GREATER_OR_EQUAL,
                always = VK_COMPARE_OP_ALWAYS,
            };
            enum class Operation
            {
                keep = VK_STENCIL_OP_KEEP,
                zero = VK_STENCIL_OP_ZERO,
                replace = VK_STENCIL_OP_REPLACE,
                increment_and_clamp = VK_STENCIL_OP_INCREMENT_AND_CLAMP,
                decrement_and_clamp = VK_STENCIL_OP_DECREMENT_AND_CLAMP,
                invert = VK_STENCIL_OP_INVERT,
                increment_and_wrap = VK_STENCIL_OP_INCREMENT_AND_WRAP,
                decrement_and_wrap = VK_STENCIL_OP_DECREMENT_AND_WRAP,
            };

            Operation fail;
            Operation pass;
            Operation depth_fail;
            Compare compare;
            std::uint32_t compare_mask;
            std::uint32_t write_mask;
            std::uint32_t reference;
        };
        bool test = false;

        State front;
        State back;
    };
    struct Multisampling
    {
        enum class SampleCount
        {
            _1 = VK_SAMPLE_COUNT_1_BIT,
            _2 = VK_SAMPLE_COUNT_2_BIT,
            _4 = VK_SAMPLE_COUNT_4_BIT,
            _8 = VK_SAMPLE_COUNT_8_BIT,
            _16 = VK_SAMPLE_COUNT_16_BIT,
            _32 = VK_SAMPLE_COUNT_32_BIT,
            _64 = VK_SAMPLE_COUNT_64_BIT,
        };

        SampleCount rasterization_samples;
        bool alpha_to_coverage = false;

        std::uint32_t sample_mask[2];
    };
    struct Blend
    {
        struct Attachment
        {
            enum class Factor : std::uint32_t
            {
                zero = VK_BLEND_FACTOR_ZERO,
                one = VK_BLEND_FACTOR_ONE,
                source = VK_BLEND_FACTOR_SRC_COLOR,
                one_minus_source = VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR,
                destination = VK_BLEND_FACTOR_DST_COLOR,
                one_minus_destination = VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR,
                source_alpha = VK_BLEND_FACTOR_SRC_ALPHA,
                one_minus_source_alpha = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
                destination_alpha = VK_BLEND_FACTOR_DST_ALPHA,
                one_minus_destination_alpha = VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA,
                constant_color = VK_BLEND_FACTOR_CONSTANT_COLOR,
                one_minus_constant_color = VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR,
                constant_alpha = VK_BLEND_FACTOR_CONSTANT_ALPHA,
                one_minus_constant_alpha = VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_ALPHA,
                source_alpha_saturate = VK_BLEND_FACTOR_SRC_ALPHA_SATURATE,
                source_1_color = VK_BLEND_FACTOR_SRC1_COLOR,
                one_minus_source_1_color = VK_BLEND_FACTOR_ONE_MINUS_SRC1_COLOR,
                source_1_alpha = VK_BLEND_FACTOR_SRC1_ALPHA,
                one_minus_source_1_alpha = VK_BLEND_FACTOR_ONE_MINUS_SRC1_ALPHA,
            };
            enum class Operator : std::uint32_t
            {
                add = VK_BLEND_OP_ADD,
                substract = VK_BLEND_OP_SUBTRACT,
                reverse_substract = VK_BLEND_OP_REVERSE_SUBTRACT,
                min = VK_BLEND_OP_MIN,
                max = VK_BLEND_OP_MAX,
                zero = VK_BLEND_OP_ZERO_EXT,
                source = VK_BLEND_OP_SRC_EXT,
                estination = VK_BLEND_OP_DST_EXT,
                source_over_destination = VK_BLEND_OP_SRC_OVER_EXT,
                destination_over_source = VK_BLEND_OP_DST_OVER_EXT,
                source_in = VK_BLEND_OP_SRC_IN_EXT,
                destination_in = VK_BLEND_OP_DST_IN_EXT,
                source_out = VK_BLEND_OP_SRC_OUT_EXT,
                destination_out = VK_BLEND_OP_DST_OUT_EXT,
                source_atop = VK_BLEND_OP_SRC_ATOP_EXT,
                destination_atop = VK_BLEND_OP_DST_ATOP_EXT,
                xor_ = VK_BLEND_OP_XOR_EXT,
                multiply = VK_BLEND_OP_MULTIPLY_EXT,
                screen = VK_BLEND_OP_SCREEN_EXT,
                overlay = VK_BLEND_OP_OVERLAY_EXT,
                darken = VK_BLEND_OP_DARKEN_EXT,
                lighten = VK_BLEND_OP_LIGHTEN_EXT,
                color_dodge = VK_BLEND_OP_COLORDODGE_EXT,
                color_burn = VK_BLEND_OP_COLORBURN_EXT,
                hard_light = VK_BLEND_OP_HARDLIGHT_EXT,
                soft_light = VK_BLEND_OP_SOFTLIGHT_EXT,
                difference = VK_BLEND_OP_DIFFERENCE_EXT,
                exclusion = VK_BLEND_OP_EXCLUSION_EXT,
                invert = VK_BLEND_OP_INVERT_EXT,
                invert_rgb = VK_BLEND_OP_INVERT_RGB_EXT,
                lienear_dodge = VK_BLEND_OP_LINEARDODGE_EXT,
                linear_burn = VK_BLEND_OP_LINEARBURN_EXT,
                vivid_light = VK_BLEND_OP_VIVIDLIGHT_EXT,
                linear_light = VK_BLEND_OP_LINEARLIGHT_EXT,
                pin_light = VK_BLEND_OP_PINLIGHT_EXT,
                hard_mix = VK_BLEND_OP_HARDMIX_EXT,
                hsl_hue = VK_BLEND_OP_HSL_HUE_EXT,
                hsl_saturation = VK_BLEND_OP_HSL_SATURATION_EXT,
                hsl_color = VK_BLEND_OP_HSL_COLOR_EXT,
                hsl_luminosity = VK_BLEND_OP_HSL_LUMINOSITY_EXT,
                plus = VK_BLEND_OP_PLUS_EXT,
                plus_clamped = VK_BLEND_OP_PLUS_CLAMPED_EXT,
                plus_clamped_alpha = VK_BLEND_OP_PLUS_CLAMPED_ALPHA_EXT,
                plus_darker = VK_BLEND_OP_PLUS_DARKER_EXT,
                minus = VK_BLEND_OP_MINUS_EXT,
                minus_clamped = VK_BLEND_OP_MINUS_CLAMPED_EXT,
                contrast = VK_BLEND_OP_CONTRAST_EXT,
                invert_openvg = VK_BLEND_OP_INVERT_OVG_EXT,
                red = VK_BLEND_OP_RED_EXT,
                green = VK_BLEND_OP_GREEN_EXT,
                blue = VK_BLEND_OP_BLUE_EXT,
            };
            enum WriteMask : std::uint32_t
            {
                r = VK_COLOR_COMPONENT_R_BIT,
                g = VK_COLOR_COMPONENT_G_BIT,
                b = VK_COLOR_COMPONENT_B_BIT,
                a = VK_COLOR_COMPONENT_A_BIT
            };

            bool enable;
            WriteMask write_mask;

            struct Color
            {
                Factor source_factor;
                Factor destination_factor;
                Operator opr;
            } color;
            struct Alpha
            {
                Factor source_factor;
                Factor destination_factor;
                Operator opr;
            } alpha;
        };

        std::span<Attachment> attachments;
        std::array<float, 4u> constants;
    };
    struct ShaderInput
    {
        struct Vertex
        {
            enum class Rate : std::uint32_t
            {
                vertex = VK_VERTEX_INPUT_RATE_VERTEX,
                instance = VK_VERTEX_INPUT_RATE_INSTANCE
            };

            struct Attribute
            {
                using Kind = lx::gpu::Format;
                using enum Kind;

                Kind kind;
                std::size_t location;
                std::size_t padding_in_bytes;
            };

            Rate rate;
            std::span<Attribute> attributes;
        };
        struct GlobalData
        {
            struct Binding
            {
                enum class Shader : std::uint32_t
                {
                    vertex = VK_SHADER_STAGE_VERTEX_BIT,
                    fragment = VK_SHADER_STAGE_FRAGMENT_BIT
                };

                enum class Kind : std::uint32_t
                {
                    sampler = VK_DESCRIPTOR_TYPE_SAMPLER,
                    combined_image_sampler = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                    sampled_image = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
                    storage_image = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
                    uniform_texel_buffer = VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER,
                    storage_texel_buffer = VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER,
                    uniform_buffer = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                    storage_buffer = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                    uniform_buffer_dynamic = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
                    storage_buffer_dynamic = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC,
                    input_attachment = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT
                };

                Kind kind;
                using enum Kind;

                Shader shader;
                std::size_t count;
                std::size_t size_in_bytes;
            };

            std::span<Binding> bindings;
        };
        struct PushConstant
        {
            enum class Shader : std::uint32_t
            {
                vertex = VK_SHADER_STAGE_VERTEX_BIT,
                fragment = VK_SHADER_STAGE_FRAGMENT_BIT
            };

            Shader shader;
            std::size_t size_in_bytes;
            std::size_t padding_in_bytes;
        };

        std::span<Vertex> vertex_layouts;
        std::span<GlobalData> global_data_layouts;
        std::span<PushConstant> push_constants;
    };
    struct Clip
    {
        Viewport viewport;
        lx::common::Rect<std::int32_t, std::uint32_t> scissors;
    };

    struct Properties
    {
        Primitive primitive;
        Depth depth;
        Stencil stencil;
        Multisampling multisampling;
        Blend blend;
        ShaderInput shader_input;
        std::size_t subpass_index;

        std::span<Shader> shaders;
        std::span<Clip> clips;

        std::span<DynamicState> dynamic_states;
    };

    bool is_created() const
    {
        return VK_NULL_HANDLE != this->vk_pipeline && VK_NULL_HANDLE != this->vk_pipeline_layout;
    }

private:
    Pipeline(VkDevice vk_device_a, const Properties& properties_a, const RenderPass& render_pass);
    void destroy(VkDevice vk_device_a);

    VkPipeline vk_pipeline = VK_NULL_HANDLE;
    VkPipelineLayout vk_pipeline_layout = VK_NULL_HANDLE;

    friend class Device;
};

constexpr Pipeline<pipeline::graphics>::Primitive::Cull operator|(Pipeline<pipeline::graphics>::Primitive::Cull left_a,
                                                                  Pipeline<pipeline::graphics>::Primitive::Cull right_a)
{
    return static_cast<Pipeline<pipeline::graphics>::Primitive::Cull>(static_cast<std::uint32_t>(left_a) |
                                                                      static_cast<std::uint32_t>(right_a));
}

constexpr Pipeline<pipeline::graphics>::Primitive::Cull operator&(Pipeline<pipeline::graphics>::Primitive::Cull left_a,
                                                                  Pipeline<pipeline::graphics>::Primitive::Cull right_a)
{
    return static_cast<Pipeline<pipeline::graphics>::Primitive::Cull>(static_cast<std::uint32_t>(left_a) &
                                                                      static_cast<std::uint32_t>(right_a));
}
} // namespace lx::gpu