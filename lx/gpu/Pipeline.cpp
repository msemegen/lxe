// this
#include <lx/gpu/Pipeline.hpp>

// lx
#include <lx/utils/logger.hpp>

namespace {
using namespace lx::gpu;

std::size_t get_format_size_in_bytes(Pipeline<pipeline::graphics>::ShaderInput::Vertex::Attribute::Kind kind_a)
{
    switch (kind_a)
    {
        case Pipeline<pipeline::graphics>::ShaderInput::Vertex::Attribute::r8_unorm:
        case Pipeline<pipeline::graphics>::ShaderInput::Vertex::Attribute::r8_snorm:
        case Pipeline<pipeline::graphics>::ShaderInput::Vertex::Attribute::r8_uint:
        case Pipeline<pipeline::graphics>::ShaderInput::Vertex::Attribute::r8_sint:
        case Pipeline<pipeline::graphics>::ShaderInput::Vertex::Attribute::r8_srgb:
            return 1;

        case Pipeline<pipeline::graphics>::ShaderInput::Vertex::Attribute::r8g8_unorm:
        case Pipeline<pipeline::graphics>::ShaderInput::Vertex::Attribute::r8g8_snorm:
        case Pipeline<pipeline::graphics>::ShaderInput::Vertex::Attribute::r8g8_uint:
        case Pipeline<pipeline::graphics>::ShaderInput::Vertex::Attribute::r8g8_sint:
        case Pipeline<pipeline::graphics>::ShaderInput::Vertex::Attribute::r8g8_srgb:
            return 2;

        case Pipeline<pipeline::graphics>::ShaderInput::Vertex::Attribute::r8g8b8_unorm:
        case Pipeline<pipeline::graphics>::ShaderInput::Vertex::Attribute::r8g8b8_snorm:
        case Pipeline<pipeline::graphics>::ShaderInput::Vertex::Attribute::r8g8b8_uint:
        case Pipeline<pipeline::graphics>::ShaderInput::Vertex::Attribute::r8g8b8_sint:
        case Pipeline<pipeline::graphics>::ShaderInput::Vertex::Attribute::r8g8b8_srgb:
        case Pipeline<pipeline::graphics>::ShaderInput::Vertex::Attribute::b8g8r8_unorm:
        case Pipeline<pipeline::graphics>::ShaderInput::Vertex::Attribute::b8g8r8_snorm:
        case Pipeline<pipeline::graphics>::ShaderInput::Vertex::Attribute::b8g8r8_uint:
        case Pipeline<pipeline::graphics>::ShaderInput::Vertex::Attribute::b8g8r8_sint:
        case Pipeline<pipeline::graphics>::ShaderInput::Vertex::Attribute::b8g8r8_srgb:
            return 3;

        case Pipeline<pipeline::graphics>::ShaderInput::Vertex::Attribute::r8g8b8a8_unorm:
        case Pipeline<pipeline::graphics>::ShaderInput::Vertex::Attribute::r8g8b8a8_snorm:
        case Pipeline<pipeline::graphics>::ShaderInput::Vertex::Attribute::r8g8b8a8_uint:
        case Pipeline<pipeline::graphics>::ShaderInput::Vertex::Attribute::r8g8b8a8_sint:
        case Pipeline<pipeline::graphics>::ShaderInput::Vertex::Attribute::r8g8b8a8_srgb:
        case Pipeline<pipeline::graphics>::ShaderInput::Vertex::Attribute::b8g8r8a8_unorm:
        case Pipeline<pipeline::graphics>::ShaderInput::Vertex::Attribute::b8g8r8a8_snorm:
        case Pipeline<pipeline::graphics>::ShaderInput::Vertex::Attribute::b8g8r8a8_uint:
        case Pipeline<pipeline::graphics>::ShaderInput::Vertex::Attribute::b8g8r8a8_sint:
        case Pipeline<pipeline::graphics>::ShaderInput::Vertex::Attribute::b8g8r8a8_srgb:
            return 4;

        case Pipeline<pipeline::graphics>::ShaderInput::Vertex::Attribute::r16_uint:
        case Pipeline<pipeline::graphics>::ShaderInput::Vertex::Attribute::r16_sint:
        case Pipeline<pipeline::graphics>::ShaderInput::Vertex::Attribute::r16_sfloat:
            return 2;

        case Pipeline<pipeline::graphics>::ShaderInput::Vertex::Attribute::r16g16_uint:
        case Pipeline<pipeline::graphics>::ShaderInput::Vertex::Attribute::r16g16_sint:
        case Pipeline<pipeline::graphics>::ShaderInput::Vertex::Attribute::r16g16_sfloat:
            return 4;

        case Pipeline<pipeline::graphics>::ShaderInput::Vertex::Attribute::r16g16b16_uint:
        case Pipeline<pipeline::graphics>::ShaderInput::Vertex::Attribute::r16g16b16_sint:
        case Pipeline<pipeline::graphics>::ShaderInput::Vertex::Attribute::r16g16b16_sfloat:
            return 6;

        case Pipeline<pipeline::graphics>::ShaderInput::Vertex::Attribute::r16g16b16a16_uint:
        case Pipeline<pipeline::graphics>::ShaderInput::Vertex::Attribute::r16g16b16a16_sint:
        case Pipeline<pipeline::graphics>::ShaderInput::Vertex::Attribute::r16g16b16a16_sfloat:
            return 8;

        case Pipeline<pipeline::graphics>::ShaderInput::Vertex::Attribute::r32_uint:
        case Pipeline<pipeline::graphics>::ShaderInput::Vertex::Attribute::r32_sint:
        case Pipeline<pipeline::graphics>::ShaderInput::Vertex::Attribute::r32_sfloat:
            return 4;

        case Pipeline<pipeline::graphics>::ShaderInput::Vertex::Attribute::r32g32_uint:
        case Pipeline<pipeline::graphics>::ShaderInput::Vertex::Attribute::r32g32_sint:
        case Pipeline<pipeline::graphics>::ShaderInput::Vertex::Attribute::r32g32_sfloat:
            return 8;

        case Pipeline<pipeline::graphics>::ShaderInput::Vertex::Attribute::r32g32b32_uint:
        case Pipeline<pipeline::graphics>::ShaderInput::Vertex::Attribute::r32g32b32_sint:
        case Pipeline<pipeline::graphics>::ShaderInput::Vertex::Attribute::r32g32b32_sfloat:
            return 12;

        case Pipeline<pipeline::graphics>::ShaderInput::Vertex::Attribute::r32g32b32a32_uint:
        case Pipeline<pipeline::graphics>::ShaderInput::Vertex::Attribute::r32g32b32a32_sint:
        case Pipeline<pipeline::graphics>::ShaderInput::Vertex::Attribute::r32g32b32a32_sfloat:
            return 16;

        case Pipeline<pipeline::graphics>::ShaderInput::Vertex::Attribute::b10g11r11_ufloat_pack32:
        case Pipeline<pipeline::graphics>::ShaderInput::Vertex::Attribute::e5b9g9r9_ufloat_pack32:
        case Pipeline<pipeline::graphics>::ShaderInput::Vertex::Attribute::a2r10g10b10_unorm_pack32:
        case Pipeline<pipeline::graphics>::ShaderInput::Vertex::Attribute::a2b10g10r10_unorm_pack32:
            return 4;

        case Pipeline<pipeline::graphics>::ShaderInput::Vertex::Attribute::r64_sfloat:
            return 8u;

        case Pipeline<pipeline::graphics>::ShaderInput::Vertex::Attribute::r64g64_sfloat:
            return 16u;

        case Pipeline<pipeline::graphics>::ShaderInput::Vertex::Attribute::r64g64b64_sfloat:
            return 24u;

        case Pipeline<pipeline::graphics>::ShaderInput::Vertex::Attribute::r64g64b64a64_sfloat:
            return 32u;
    }

    return 0u;
}
} // namespace

namespace lx::gpu {
using namespace lx::containers;

Pipeline<pipeline::graphics>::Pipeline(VkDevice vk_device_a, const Properties& properties_a, const RenderPass& render_pass_a)
{
    Vector<VkPipelineShaderStageCreateInfo> vk_shader_stage_create_infos(properties_a.shaders.size());

    Vector<VkVertexInputAttributeDescription> vk_vertex_input_attribute_descriptions;
    Vector<VkVertexInputBindingDescription> vk_vertex_input_binding_descriptions;

    Vector<VkDescriptorSetLayout> vk_descriptor_set_layouts;
    Vector<VkPushConstantRange> vk_push_constant_ranges;

    if (false == properties_a.shader_input.vertex_layouts.empty())
    {
        vk_vertex_input_binding_descriptions.reserve(properties_a.shader_input.vertex_layouts.size());

        std::uint32_t vertex_binding_index = 0;
        for (const auto& vertex_layout : properties_a.shader_input.vertex_layouts)
        {
            std::size_t stride = 0u;
            for (const auto& vertex_attribute : vertex_layout.attributes)
            {
                vk_vertex_input_attribute_descriptions.emplace_back(static_cast<std::uint32_t>(vertex_attribute.location),
                                                                    vertex_binding_index,
                                                                    static_cast<VkFormat>(vertex_attribute.kind),
                                                                    static_cast<std::uint32_t>(stride));
                stride += get_format_size_in_bytes(vertex_attribute.kind) + vertex_attribute.padding_in_bytes;
            }
            vk_vertex_input_binding_descriptions.emplace_back(
                vertex_binding_index, static_cast<std::uint32_t>(stride), static_cast<VkVertexInputRate>(vertex_layout.rate));
            vertex_binding_index++;
        }
    }

    if (false == properties_a.shader_input.global_data_layouts.empty())
    {
        vk_descriptor_set_layouts.reserve(properties_a.shader_input.global_data_layouts.size());

        for (const auto& global_data_layout : properties_a.shader_input.global_data_layouts)
        {
            std::uint32_t descriptor_set_binding_index = 0;
            Vector<VkDescriptorSetLayoutBinding> vk_descriptor_set_layout_bindings;

            for (const auto& binding : global_data_layout.bindings)
            {
                vk_descriptor_set_layout_bindings.emplace_back(descriptor_set_binding_index,
                                                               static_cast<VkDescriptorType>(binding.kind),
                                                               static_cast<std::uint32_t>(binding.count),
                                                               static_cast<VkShaderStageFlags>(binding.shader));
                descriptor_set_binding_index++;
            }

            VkDescriptorSetLayoutCreateInfo vk_descriptor_set_layout_create_info {
                .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
                .pNext = nullptr,
                .flags = 0x0u,
                .bindingCount = static_cast<std::uint32_t>(vk_descriptor_set_layout_bindings.get_length()),
                .pBindings = vk_descriptor_set_layout_bindings.get_buffer()
            };

            VkDescriptorSetLayout vk_descriptor_set_layout;

            if (VK_SUCCESS ==
                vkCreateDescriptorSetLayout(vk_device_a, &vk_descriptor_set_layout_create_info, nullptr, &vk_descriptor_set_layout))
            {
                vk_descriptor_set_layouts.push_back(vk_descriptor_set_layout);
            }
        }
    }

    if (false == properties_a.shader_input.push_constants.empty())
    {
        std::size_t offset = 0u;
        vk_push_constant_ranges.reserve(properties_a.shader_input.push_constants.size());

        for (const auto& push_constant : properties_a.shader_input.push_constants)
        {
            VkPushConstantRange vk_push_constant_range { .stageFlags = static_cast<VkShaderStageFlags>(push_constant.shader),
                                                         .offset = static_cast<std::uint32_t>(offset),
                                                         .size = static_cast<std::uint32_t>(push_constant.size_in_bytes) };

            vk_push_constant_ranges.push_back(vk_push_constant_ranges);

            offset += push_constant.size_in_bytes + push_constant.padding_in_bytes;
        }
    }

    for (const auto& shader : properties_a.shaders)
    {
        vk_shader_stage_create_infos.emplace_back(VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                                                  nullptr,
                                                  0x0u,
                                                  static_cast<VkShaderStageFlagBits>(shader.get_properties().kind),
                                                  shader,
                                                  shader.get_properties().entry_point_name.data(),
                                                  nullptr);
    }

    VkDynamicState vk_dynamic_states[] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
    VkPipelineDynamicStateCreateInfo vk_dynamic_state_create_info { .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
                                                                    .pNext = nullptr,
                                                                    .flags = 0x0u,
                                                                    .dynamicStateCount = _countof(vk_dynamic_states),
                                                                    .pDynamicStates = vk_dynamic_states };

    VkPipelineViewportStateCreateInfo vk_viewport_state { .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
                                                          .flags = 0x0u,
                                                          .viewportCount = static_cast<std::uint32_t>(properties_a.clips.size()),
                                                          .pViewports = nullptr,
                                                          .scissorCount = static_cast<std::uint32_t>(properties_a.clips.size()),
                                                          .pScissors = nullptr };

    VkPipelineVertexInputStateCreateInfo vk_pipeline_vertex_input_state_create_info {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0u,
        .vertexBindingDescriptionCount = static_cast<std::uint32_t>(properties_a.shader_input.vertex_layouts.size()),
        .pVertexBindingDescriptions = vk_vertex_input_binding_descriptions.get_buffer(),
        .vertexAttributeDescriptionCount = static_cast<std::uint32_t>(vk_vertex_input_attribute_descriptions.get_length()),
        .pVertexAttributeDescriptions = vk_vertex_input_attribute_descriptions.get_buffer()
    };

    VkPipelineInputAssemblyStateCreateInfo vk_pipeline_input_assembly_state_create_info {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0u,
        .topology = static_cast<VkPrimitiveTopology>(properties_a.primitive.topology),
        .primitiveRestartEnable = VK_FALSE
    };

    VkPipelineColorBlendStateCreateInfo vk_pipeline_color_blend_state_create_info {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0x0u,
        .logicOpEnable = VK_FALSE,
    };

    VkPipelineTessellationStateCreateInfo vk_pipeline_tessallation_state_create_info {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO, .pNext = nullptr, .flags = 0x0u, .patchControlPoints = 3u
    };

    VkPipelineRasterizationStateCreateInfo vk_pipeline_rasterization_state_create_info {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0x0u,
        .depthClampEnable = VK_FALSE,
        .rasterizerDiscardEnable = VK_FALSE,
        .polygonMode = static_cast<VkPolygonMode>(properties_a.primitive.fill),
        .cullMode = static_cast<VkCullModeFlags>(properties_a.primitive.cull),
        .frontFace = static_cast<VkFrontFace>(properties_a.primitive.front_face),
        .depthBiasEnable = (0.0f == properties_a.depth.bias_constant_factor ? VK_FALSE : VK_TRUE),
        .depthBiasConstantFactor = properties_a.depth.bias_constant_factor,
        .depthBiasClamp = 0.0f,
        .depthBiasSlopeFactor = properties_a.depth.bias_slope_factor,
        .lineWidth = 1.0f
    };

    VkPipelineMultisampleStateCreateInfo vk_pipeline_multisample_state_create_info {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0x0u,
        .rasterizationSamples = static_cast<VkSampleCountFlagBits>(properties_a.multisampling.rasterization_samples),
        .sampleShadingEnable = VK_FALSE,
        .minSampleShading = 1.0f,
        .pSampleMask = properties_a.multisampling.sample_mask,
        .alphaToCoverageEnable = true == properties_a.multisampling.alpha_to_coverage ? VK_TRUE : VK_FALSE,
        .alphaToOneEnable = VK_FALSE
    };

    VkPipelineLayoutCreateInfo vk_pipeline_layout_create_info { .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
                                                                .pNext = nullptr,
                                                                .flags = 0x0u,
                                                                .setLayoutCount =
                                                                    static_cast<std::uint32_t>(vk_descriptor_set_layouts.get_length()),
                                                                .pSetLayouts = vk_descriptor_set_layouts.get_buffer(),
                                                                .pushConstantRangeCount =
                                                                    static_cast<std::uint32_t>(vk_push_constant_ranges.get_length()),
                                                                .pPushConstantRanges = vk_push_constant_ranges.get_buffer() };

    auto res = vkCreatePipelineLayout(vk_device_a, &vk_pipeline_layout_create_info, nullptr, &(this->vk_pipeline_layout));

    if (VK_SUCCESS != res && VK_NULL_HANDLE != this->vk_pipeline_layout)
    {
        log_err("vkCreatePipelineLayout failed: {}", static_cast<std::underlying_type_t<decltype(res)>>(res));
    }
    else
    {
        VkGraphicsPipelineCreateInfo vk_graphics_pipeline_create_info {
            .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0u,
            .stageCount = static_cast<std::uint32_t>(vk_shader_stage_create_infos.get_length()),
            .pStages = vk_shader_stage_create_infos.get_buffer(),
            .pVertexInputState = &vk_pipeline_vertex_input_state_create_info,
            .pInputAssemblyState = &vk_pipeline_input_assembly_state_create_info,
            .pTessellationState = &vk_pipeline_tessallation_state_create_info,
            .pViewportState = &vk_viewport_state,
            .pRasterizationState = &vk_pipeline_rasterization_state_create_info,
            .pMultisampleState = &vk_pipeline_multisample_state_create_info,
            .pColorBlendState = &vk_pipeline_color_blend_state_create_info,
            .pDynamicState = &vk_dynamic_state_create_info,
            .layout = this->vk_pipeline_layout,
            .renderPass = render_pass_a,
            .subpass = static_cast<std::uint32_t>(properties_a.subpass_index)
        };

        res = vkCreateGraphicsPipelines(vk_device_a, VK_NULL_HANDLE, 1u, &vk_graphics_pipeline_create_info, nullptr, &(this->vk_pipeline));

        if (VK_SUCCESS != res || VK_NULL_HANDLE == this->vk_pipeline)
        {
            log_err("vkCreateGraphicsPipelines failed: {}", static_cast<std::underlying_type_t<decltype(res)>>(res));
        }
    }
}

void Pipeline<pipeline::graphics>::destroy(VkDevice vk_device_a)
{
    vkDestroyPipeline(vk_device_a, this->vk_pipeline, nullptr);
}
} // namespace lx::gpu