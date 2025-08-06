// this
#include <lx/app.hpp>

// lx
#include <lx/containers/Vector.hpp>
#include <lx/utils/logger.hpp>

// externals
#include <lxl/lxl.h>

// win32
#include <Windows.h>

// std
#include <bit>
#include <string>
#include <vector>

VkInstance vk_instance;
lx::Windower windower;

namespace {
using namespace lx::common;
using namespace lx::containers;
using namespace lx::devices;
using namespace lx::utils;

BOOL enum_monitors_handler(HMONITOR monitor_handle, HDC, LPRECT, LPARAM user_data)
{
    auto* displays = std::bit_cast<Vector<Display>*>(user_data);

    MONITORINFOEX monitor_info;
    DEVMODE dev_mode;

    monitor_info.cbSize = sizeof(monitor_info);
    dev_mode.dmSize = sizeof(dev_mode);

    GetMonitorInfo(monitor_handle, &monitor_info);
    EnumDisplaySettings(monitor_info.szDevice, ENUM_CURRENT_SETTINGS, &dev_mode);

    displays->emplace_back(monitor_handle,
                           0 == monitor_info.rcMonitor.left && 0 == monitor_info.rcMonitor.top,
                           static_cast<std::uint8_t>(dev_mode.dmBitsPerPel),
                           reinterpret_cast<const char*>(dev_mode.dmDeviceName),
                           Rect<std::int64_t, std::uint64_t> {
                               .position = { .x = monitor_info.rcMonitor.left, .y = monitor_info.rcMonitor.top },
                               .size = { .w = static_cast<std::uint64_t>(monitor_info.rcMonitor.right - monitor_info.rcMonitor.left),
                                         .h = static_cast<std::uint64_t>(monitor_info.rcMonitor.bottom - monitor_info.rcMonitor.top) } },
                           Rect<std::int64_t, std::uint64_t> { .position = { .x = dev_mode.dmPosition.x, .y = dev_mode.dmPosition.y },
                                                               .size = { .w = dev_mode.dmPosition.x + dev_mode.dmPelsWidth,
                                                                         .h = dev_mode.dmPosition.y + dev_mode.dmPelsHeight } });

    return TRUE;
}

VKAPI_ATTR VkBool32 VKAPI_CALL vk_debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT message_severity_a,
                                                 VkDebugUtilsMessageTypeFlagsEXT message_type_a,
                                                 const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                                                 void*)
{
    auto severity_to_string = [&]() -> const char* {
        switch (message_severity_a)
        {
            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT: {
                return "sev_vk_inf";
            }

            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT: {
                return "sev_vk_wrn";
            }

            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT: {
                return "sev_vk_err";
            }

            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT: {
                return "sev_vk_veb";
            }
        }

        return "";
    };

    auto type_to_string = [&]() -> const char* {
        switch (message_type_a)
        {
            case VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT: {
                return "type_vk_gen";
            }

            case VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT: {
                return "type_vk_vld";
            }

            case VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT: {
                return "type_vk_pef";
            }

            case VK_DEBUG_UTILS_MESSAGE_TYPE_DEVICE_ADDRESS_BINDING_BIT_EXT: {
                return "type_vk_add";
            }
        }

        return "";
    };

    log_dbg("[{}][{}] {}", severity_to_string(), type_to_string(), pCallbackData->pMessage);
    return VK_FALSE;
}

constexpr std::string_view engine_name = "lx";
constexpr Version engine_version = Version::Components { .major = 0u, .minor = 0u, .patch = 1u };
constexpr Version vulkan_version = Version::Components { .major = 1u, .minor = 1u, .patch = 0u };

FILE* p_log_file = nullptr;
bool log_console_output = false;
} // namespace

namespace lx::utils {
using namespace lx::containers;

void logger::Composer::begin(Kind kind_a, const std::source_location& source_location_a)
{
    if (static_cast<std::uint64_t>(kind_a) == (static_cast<std::uint64_t>(logger::kind) & static_cast<std::uint64_t>(kind_a)))
    {
        this->kind = kind_a;

        String<char, 34u> buffer;
        buffer.reserve(21);

        auto now = std::chrono::floor<std::chrono::seconds>(std::chrono::system_clock::now());
        auto local_time = std::chrono::zoned_time { std::chrono::current_zone(), now };

        std::format_to(buffer.get_buffer(), "[{:%H:%M:%S %d.%m.%Y}]", local_time);

        switch (this->kind)
        {
            case Kind::inf:
                buffer.push_back("[inf]");
                break;
            case Kind::wrn:
                buffer.push_back("[wrn]");
                break;
            case Kind::err:
                buffer.push_back("[err]");
                break;
            case Kind::omg:
                buffer.push_back("[omg]");
                break;
            case Kind::dbg:
                buffer.push_back("[dbg]");
                break;
        }

        logger::log(buffer.get_cstring());
        logger::log(std::format("[{}][{},{}] ", source_location_a.file_name(), source_location_a.line(), source_location_a.column()));
    }
}
void logger::Composer::end()
{
    if (static_cast<std::uint64_t>(this->kind) == (static_cast<std::uint64_t>(logger::kind) & static_cast<std::uint64_t>(this->kind)))
    {
        logger::log("\n");
        fflush(p_log_file);
        if (true == log_console_output)
        {
            fflush(stdout);
        }
    }
}

void logger::log(std::string_view log_a)
{
    std::print(p_log_file, "{}", log_a);

    if (true == log_console_output)
    {
        std::print(stdout, "{}", log_a);
    }
}
} // namespace lx::utils

int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR cmd_line, _In_ int)
{
    using namespace lx::common;
    using namespace lx::containers;
    using namespace lx::devices;
    using namespace lx::gpu;
    using namespace lx::utils;

    Vector<Display> displays;
    Vector<GPU> gpus;

    lx::app::Config config;
    lx::app::setup(out(config));

    if (true == config.log.path.is_empty())
    {
        // log filename empty error code
        return -1;
    }

    errno_t file_err = fopen_s(&p_log_file, config.log.path.get_cstring(), "w+");

    if (0 != file_err)
    {
        return -2;
    }

    if (true == config.log.console)
    {
        log_console_output = static_cast<bool>(AllocConsole());

        if (true == log_console_output)
        {
            FILE* fp1 = nullptr;
            FILE* fp2 = nullptr;
            FILE* fp3 = nullptr;

            freopen_s(&fp1, "CONIN$", "r", stdin);
            freopen_s(&fp2, "CONOUT$", "w", stdout);
            freopen_s(&fp3, "CONOUT$", "w", stderr);
        }
    }

    std::uint32_t loaded_vulkan_version = lxlCreate();
    bool vulkan_initialized = false;

    if (0u != loaded_vulkan_version)
    {
        Vector<const char*, 4u> default_instance_extensions;
        Vector<const char*, 3u> default_instance_layers;

        default_instance_extensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
        default_instance_extensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#if defined(VK_KHR_get_physical_device_properties2)
        default_instance_extensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
#endif

        if (true == config.vulkan.enable_validation)
        {
            default_instance_extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
            default_instance_extensions.push_back(VK_EXT_VALIDATION_FEATURES_EXTENSION_NAME);
            default_instance_layers.push_back("VK_LAYER_KHRONOS_validation");
        }

        const VkApplicationInfo application_info { .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
                                                   .pNext = nullptr,
                                                   .pApplicationName = config.app.name.get_cstring(),
                                                   .applicationVersion = config.app.version,
                                                   .pEngineName = engine_name.data(),
                                                   .engineVersion = engine_version,
                                                   .apiVersion = vulkan_version };

        VkDebugUtilsMessengerCreateInfoEXT debug_messenger_create_info {
            .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
            .pNext = nullptr,
            .flags = 0x0u,
            .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
            .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT,

            .pfnUserCallback = vk_debug_callback,
            .pUserData = nullptr
        };

        const VkInstanceCreateInfo vk_instance_create_info {
            .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
            .pNext = (true == config.vulkan.enable_validation ? &debug_messenger_create_info : nullptr),
            .flags = 0x0u,
            .pApplicationInfo = &application_info,
            .enabledLayerCount = static_cast<std::uint32_t>(default_instance_layers.get_length()),
            .ppEnabledLayerNames = default_instance_layers.get_buffer(),
            .enabledExtensionCount = static_cast<std::uint32_t>(default_instance_extensions.get_length()),
            .ppEnabledExtensionNames = default_instance_extensions.get_buffer()
        };

        VkResult vk_result = vkCreateInstance(&vk_instance_create_info, nullptr, &vk_instance);
        vulkan_initialized = VK_SUCCESS == vk_result;

        if (true == vulkan_initialized)
        {
            VkDebugUtilsMessengerEXT vk_debug_messenger = VK_NULL_HANDLE;
            if (true == config.vulkan.enable_validation)
            {
                vkCreateDebugUtilsMessengerEXT(vk_instance, &debug_messenger_create_info, nullptr, &vk_debug_messenger);
            }

            /// enum displays
            {
                EnumDisplayMonitors(nullptr, nullptr, enum_monitors_handler, reinterpret_cast<LPARAM>(&displays));
            }

            /// find primary display adapter name
            std::string primary_display_device_name;
            {
                DISPLAY_DEVICE win32_display_device;
                bool found = false;
                win32_display_device.cb = sizeof(win32_display_device);

                for (DWORD dev_index = 0u; false == found && TRUE == EnumDisplayDevices(nullptr, dev_index, &win32_display_device, 0u);
                     dev_index++)
                {
                    found = DISPLAY_DEVICE_PRIMARY_DEVICE == (win32_display_device.StateFlags & DISPLAY_DEVICE_PRIMARY_DEVICE);

                    if (true == found)
                    {
                        primary_display_device_name = win32_display_device.DeviceString;
                    }
                }

                if (false == found)
                {
                    logger::write_line(logger::omg, std::source_location::current(), "No primary display adapter found.");
                }

                /// enum GPUs
                {
                    auto from_VkPhysicalDeviceType = [](VkPhysicalDeviceType type_a) -> GPU::Kind {
                        switch (type_a)
                        {
                            case VK_PHYSICAL_DEVICE_TYPE_CPU:
                                return GPU::Kind::software;

                            case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
                                return GPU::Kind::discrete;

                            case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
                                return GPU::Kind::integrated;

                            case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
                                return GPU::Kind::indirect;
                        }

                        __assume(false);
                    };
                    auto from_VkPhysicalDeviceFeatures = [](const VkPhysicalDeviceFeatures& features_a) -> GPU::Feature {
                        GPU::Feature ret = GPU::Feature { 0x0ull };

                        if (VK_TRUE == features_a.robustBufferAccess)
                        {
                            ret |= GPU::Feature::robust_buffer_access;
                        }
                        if (VK_TRUE == features_a.fullDrawIndexUint32)
                        {
                            ret |= GPU::Feature::full_draw_index_uint32;
                        }
                        if (VK_TRUE == features_a.imageCubeArray)
                        {
                            ret |= GPU::Feature::image_cube_array;
                        }
                        if (VK_TRUE == features_a.independentBlend)
                        {
                            ret |= GPU::Feature::independent_blend;
                        }
                        if (VK_TRUE == features_a.geometryShader)
                        {
                            ret |= GPU::Feature::geometry_shader;
                        }
                        if (VK_TRUE == features_a.tessellationShader)
                        {
                            ret |= GPU::Feature::tessellation_shader;
                        }
                        if (VK_TRUE == features_a.sampleRateShading)
                        {
                            ret |= GPU::Feature::sample_rate_shading;
                        }
                        if (VK_TRUE == features_a.dualSrcBlend)
                        {
                            ret |= GPU::Feature::dual_src_blend;
                        }
                        if (VK_TRUE == features_a.logicOp)
                        {
                            ret |= GPU::Feature::logic_op;
                        }
                        if (VK_TRUE == features_a.multiDrawIndirect)
                        {
                            ret |= GPU::Feature::multi_draw_indirect;
                        }
                        if (VK_TRUE == features_a.drawIndirectFirstInstance)
                        {
                            ret |= GPU::Feature::draw_indirect_first_instance;
                        }
                        if (VK_TRUE == features_a.depthClamp)
                        {
                            ret |= GPU::Feature::depth_clamp;
                        }
                        if (VK_TRUE == features_a.depthBiasClamp)
                        {
                            ret |= GPU::Feature::depth_bias_clamp;
                        }
                        if (VK_TRUE == features_a.fillModeNonSolid)
                        {
                            ret |= GPU::Feature::fill_mode_non_nolid;
                        }
                        if (VK_TRUE == features_a.depthBounds)
                        {
                            ret |= GPU::Feature::depth_bounds;
                        }
                        if (VK_TRUE == features_a.wideLines)
                        {
                            ret |= GPU::Feature::wide_lines;
                        }
                        if (VK_TRUE == features_a.largePoints)
                        {
                            ret |= GPU::Feature::large_points;
                        }
                        if (VK_TRUE == features_a.alphaToOne)
                        {
                            ret |= GPU::Feature::alpha_to_one;
                        }
                        if (VK_TRUE == features_a.multiViewport)
                        {
                            ret |= GPU::Feature::multi_viewport;
                        }
                        if (VK_TRUE == features_a.samplerAnisotropy)
                        {
                            ret |= GPU::Feature::sampler_anisotropy;
                        }
                        if (VK_TRUE == features_a.textureCompressionETC2)
                        {
                            ret |= GPU::Feature::texture_compression_ETC2;
                        }
                        if (VK_TRUE == features_a.textureCompressionASTC_LDR)
                        {
                            ret |= GPU::Feature::texture_compression_ASTC_LDR;
                        }
                        if (VK_TRUE == features_a.textureCompressionBC)
                        {
                            ret |= GPU::Feature::texture_compression_BC;
                        }
                        if (VK_TRUE == features_a.occlusionQueryPrecise)
                        {
                            ret |= GPU::Feature::occlusion_query_precise;
                        }
                        if (VK_TRUE == features_a.pipelineStatisticsQuery)
                        {
                            ret |= GPU::Feature::pipeline_statistics_query;
                        }
                        if (VK_TRUE == features_a.vertexPipelineStoresAndAtomics)
                        {
                            ret |= GPU::Feature::vertex_pipeline_stores_andatomics;
                        }
                        if (VK_TRUE == features_a.fragmentStoresAndAtomics)
                        {
                            ret |= GPU::Feature::fragment_stores_and_atomics;
                        }
                        if (VK_TRUE == features_a.shaderTessellationAndGeometryPointSize)
                        {
                            ret |= GPU::Feature::shader_tessellation_and_geometry_point_size;
                        }
                        if (VK_TRUE == features_a.shaderImageGatherExtended)
                        {
                            ret |= GPU::Feature::shader_image_gather_extended;
                        }
                        if (VK_TRUE == features_a.shaderStorageImageExtendedFormats)
                        {
                            ret |= GPU::Feature::shader_storage_image_extended_formats;
                        }
                        if (VK_TRUE == features_a.shaderStorageImageMultisample)
                        {
                            ret |= GPU::Feature::shader_storage_image_multisample;
                        }
                        if (VK_TRUE == features_a.shaderStorageImageReadWithoutFormat)
                        {
                            ret |= GPU::Feature::shader_storage_image_read_without_format;
                        }
                        if (VK_TRUE == features_a.shaderStorageImageWriteWithoutFormat)
                        {
                            ret |= GPU::Feature::shader_storage_image_write_without_format;
                        }
                        if (VK_TRUE == features_a.shaderUniformBufferArrayDynamicIndexing)
                        {
                            ret |= GPU::Feature::shader_uniform_buffer_array_dynamic_indexing;
                        }
                        if (VK_TRUE == features_a.shaderSampledImageArrayDynamicIndexing)
                        {
                            ret |= GPU::Feature::shader_storage_buffer_array_dynamic_indexing;
                        }
                        if (VK_TRUE == features_a.shaderStorageBufferArrayDynamicIndexing)
                        {
                            ret |= GPU::Feature::shader_storage_buffer_array_dynamic_indexing;
                        }
                        if (VK_TRUE == features_a.shaderStorageImageArrayDynamicIndexing)
                        {
                            ret |= GPU::Feature::shader_storage_image_array_dynamic_indexing;
                        }
                        if (VK_TRUE == features_a.shaderClipDistance)
                        {
                            ret |= GPU::Feature::shader_clip_distance;
                        }
                        if (VK_TRUE == features_a.shaderCullDistance)
                        {
                            ret |= GPU::Feature::shader_cull_distance;
                        }
                        if (VK_TRUE == features_a.shaderFloat64)
                        {
                            ret |= GPU::Feature::shader_float64;
                        }
                        if (VK_TRUE == features_a.shaderInt64)
                        {
                            ret |= GPU::Feature::shader_int64;
                        }
                        if (VK_TRUE == features_a.shaderInt16)
                        {
                            ret |= GPU::Feature::shader_int16;
                        }
                        if (VK_TRUE == features_a.shaderResourceResidency)
                        {
                            ret |= GPU::Feature::shader_resource_residency;
                        }
                        if (VK_TRUE == features_a.shaderResourceMinLod)
                        {
                            ret |= GPU::Feature::shader_resource_min_lod;
                        }
                        if (VK_TRUE == features_a.sparseBinding)
                        {
                            ret |= GPU::Feature::sparse_binding;
                        }
                        if (VK_TRUE == features_a.sparseResidencyBuffer)
                        {
                            ret |= GPU::Feature::sparse_residency_buffer;
                        }
                        if (VK_TRUE == features_a.sparseResidencyImage2D)
                        {
                            ret |= GPU::Feature::sparse_residency_image2d;
                        }
                        if (VK_TRUE == features_a.sparseResidencyImage3D)
                        {
                            ret |= GPU::Feature::sparse_residency_image3d;
                        }
                        if (VK_TRUE == features_a.sparseResidency2Samples)
                        {
                            ret |= GPU::Feature::sparse_residency_2_samples;
                        }
                        if (VK_TRUE == features_a.sparseResidency4Samples)
                        {
                            ret |= GPU::Feature::sparse_residency_4_samples;
                        }
                        if (VK_TRUE == features_a.sparseResidency8Samples)
                        {
                            ret |= GPU::Feature::sparse_residency_8_samples;
                        }
                        if (VK_TRUE == features_a.sparseResidency16Samples)
                        {
                            ret |= GPU::Feature::sparse_residency_16_samples;
                        }
                        if (VK_TRUE == features_a.sparseResidencyAliased)
                        {
                            ret |= GPU::Feature::sparse_residency_aliased;
                        }
                        if (VK_TRUE == features_a.variableMultisampleRate)
                        {
                            ret |= GPU::Feature::variable_multisample_rate;
                        }
                        if (VK_TRUE == features_a.inheritedQueries)
                        {
                            ret |= GPU::Feature::inherited_queries;
                        }

                        return ret;
                    };
                    auto from_VkPhysicalDeviceLimits = [](const VkPhysicalDeviceLimits& limits_a) -> GPU::Limits {
                        return { .max_image_dimension_1d = limits_a.maxImageDimension1D,
                                 .max_image_dimension_2d = limits_a.maxImageDimension2D,
                                 .max_image_dimension_3d = limits_a.maxImageDimension3D,
                                 .max_image_dimensioncube = limits_a.maxImageDimensionCube,
                                 .max_image_arraylayers = limits_a.maxImageArrayLayers,
                                 .max_texel_buffer_elements = limits_a.maxTexelBufferElements,
                                 .max_uniform_buffer_range = limits_a.maxUniformBufferRange,
                                 .max_storage_buffer_range = limits_a.maxStorageBufferRange,
                                 .max_push_constants_size = limits_a.maxPushConstantsSize,
                                 .max_memory_allocation_count = limits_a.maxMemoryAllocationCount,
                                 .max_sampler_allocation_count = limits_a.maxSamplerAllocationCount,
                                 .buffer_image_granularity = limits_a.bufferImageGranularity,
                                 .sparse_address_space_size = limits_a.sparseAddressSpaceSize,
                                 .max_bound_descriptor_sets = limits_a.maxBoundDescriptorSets,
                                 .max_per_stage_descriptor_samplers = limits_a.maxPerStageDescriptorSamplers,
                                 .max_per_stage_descriptor_uniform_buffers = limits_a.maxPerStageDescriptorUniformBuffers,
                                 .max_per_stage_descriptor_storage_buffers = limits_a.maxPerStageDescriptorStorageBuffers,
                                 .max_per_stage_descriptor_sampled_images = limits_a.maxPerStageDescriptorSampledImages,
                                 .max_per_stage_descriptor_storage_images = limits_a.maxPerStageDescriptorStorageImages,
                                 .max_per_stage_descriptor_input_attachments = limits_a.maxPerStageDescriptorInputAttachments,
                                 .max_per_stage_resources = limits_a.maxPerStageResources,
                                 .max_descriptor_set_samplers = limits_a.maxDescriptorSetSamplers,
                                 .max_descriptor_set_uniform_buffers = limits_a.maxDescriptorSetUniformBuffers,
                                 .max_descriptor_set_uniform_buffers_dynamic = limits_a.maxDescriptorSetUniformBuffersDynamic,
                                 .max_descriptor_set_storage_buffers = limits_a.maxDescriptorSetStorageBuffers,
                                 .max_descriptor_set_storage_buffers_dynamic = limits_a.maxDescriptorSetStorageBuffersDynamic,
                                 .max_descriptor_set_sampled_images = limits_a.maxDescriptorSetSampledImages,
                                 .max_descriptor_set_storage_images = limits_a.maxDescriptorSetStorageImages,
                                 .max_descriptor_set_input_attachments = limits_a.maxDescriptorSetInputAttachments,
                                 .max_vertex_input_attributes = limits_a.maxVertexInputAttributes,
                                 .max_vertex_input_bindings = limits_a.maxVertexInputBindings,
                                 .max_vertex_input_attribute_offset = limits_a.maxVertexInputAttributeOffset,
                                 .max_vertex_input_binding_stride = limits_a.maxVertexInputBindingStride,
                                 .max_vertex_output_components = limits_a.maxVertexOutputComponents,
                                 .max_tessellation_generationlevel = limits_a.maxTessellationGenerationLevel,
                                 .max_tessellation_patchsize = limits_a.maxTessellationPatchSize,
                                 .max_tessellation_control_per_vertex_input_components =
                                     limits_a.maxTessellationControlPerVertexInputComponents,
                                 .max_tessellation_control_per_vertex_output_components =
                                     limits_a.maxTessellationControlPerVertexOutputComponents,
                                 .max_tessellation_control_per_patch_output_components =
                                     limits_a.maxTessellationControlPerPatchOutputComponents,
                                 .max_tessellation_control_total_output_components = limits_a.maxTessellationControlTotalOutputComponents,
                                 .max_tessellation_evaluation_input_components = limits_a.maxTessellationEvaluationInputComponents,
                                 .max_tessellation_evaluation_output_components = limits_a.maxTessellationEvaluationOutputComponents,
                                 .max_geometry_shader_invocations = limits_a.maxGeometryShaderInvocations,
                                 .max_geometry_input_components = limits_a.maxGeometryInputComponents,
                                 .max_geometry_output_components = limits_a.maxGeometryOutputComponents,
                                 .max_geometry_output_vertices = limits_a.maxGeometryOutputVertices,
                                 .max_geometry_total_output_components = limits_a.maxGeometryTotalOutputComponents,
                                 .max_fragment_input_components = limits_a.maxFragmentInputComponents,
                                 .max_fragment_output_attachments = limits_a.maxFragmentOutputAttachments,
                                 .max_fragment_dual_src_attachments = limits_a.maxFragmentDualSrcAttachments,
                                 .max_fragment_combined_output_resources = limits_a.maxFragmentCombinedOutputResources,
                                 .max_compute_shared_memory_size = limits_a.maxComputeSharedMemorySize,
                                 .max_compute_work_group_count = { limits_a.maxComputeWorkGroupCount[0],
                                                                   limits_a.maxComputeWorkGroupCount[1],
                                                                   limits_a.maxComputeWorkGroupCount[2] },
                                 .max_compute_work_group_invocations = limits_a.maxComputeWorkGroupInvocations,
                                 .max_compute_work_group_size = { limits_a.maxComputeWorkGroupSize[0],
                                                                  limits_a.maxComputeWorkGroupSize[1],
                                                                  limits_a.maxComputeWorkGroupSize[2] },
                                 .sub_pixel_precision_bits = limits_a.subPixelPrecisionBits,
                                 .sub_texel_precision_bits = limits_a.subTexelPrecisionBits,
                                 .mipmap_precision_bits = limits_a.mipmapPrecisionBits,
                                 .max_draw_indexed_index_value = limits_a.maxDrawIndexedIndexValue,
                                 .max_draw_indirect_count = limits_a.maxDrawIndirectCount,
                                 .max_sampler_lod_bias = limits_a.maxSamplerLodBias,
                                 .max_sampler_anisotropy = limits_a.maxSamplerAnisotropy,
                                 .max_viewports = limits_a.maxViewports,
                                 .max_viewport_dimensions = { limits_a.maxViewportDimensions[0], limits_a.maxViewportDimensions[1] },
                                 .viewport_bounds_range = { limits_a.viewportBoundsRange[0], limits_a.viewportBoundsRange[1] },
                                 .viewport_sub_pixel_bits = limits_a.viewportSubPixelBits,
                                 .min_memory_map_alignment = limits_a.minMemoryMapAlignment,
                                 .min_texel_buffer_offset_alignment = limits_a.minTexelBufferOffsetAlignment,
                                 .min_uniform_buffer_offset_alignment = limits_a.minUniformBufferOffsetAlignment,
                                 .min_storage_buffer_offset_alignment = limits_a.minStorageBufferOffsetAlignment,
                                 .min_texel_offset = limits_a.minTexelOffset,
                                 .max_texel_offset = limits_a.maxTexelOffset,
                                 .min_texel_gather_offset = limits_a.minTexelGatherOffset,
                                 .max_texel_gather_offset = limits_a.maxTexelGatherOffset,
                                 .min_interpolation_offset = limits_a.minInterpolationOffset,
                                 .max_interpolation_offset = limits_a.maxInterpolationOffset,
                                 .sub_pixel_interpolation_offset_bits = limits_a.subPixelInterpolationOffsetBits,
                                 .max_framebuffer_width = limits_a.maxFramebufferWidth,
                                 .max_framebuffer_height = limits_a.maxFramebufferHeight,
                                 .max_framebuffer_layers = limits_a.maxFramebufferLayers,
                                 .framebuffer_color_sample_counts =
                                     static_cast<GPU::Limits::Sample_count>(limits_a.framebufferColorSampleCounts),
                                 .framebuffer_depth_sample_counts =
                                     static_cast<GPU::Limits::Sample_count>(limits_a.framebufferDepthSampleCounts),
                                 .framebuffer_stencil_sample_counts =
                                     static_cast<GPU::Limits::Sample_count>(limits_a.framebufferStencilSampleCounts),
                                 .framebuffer_no_attachments_sample_counts =
                                     static_cast<GPU::Limits::Sample_count>(limits_a.framebufferNoAttachmentsSampleCounts),
                                 .max_color_attachments = limits_a.maxColorAttachments,
                                 .sampled_image_color_sample_counts =
                                     static_cast<GPU::Limits::Sample_count>(limits_a.sampledImageColorSampleCounts),
                                 .sampled_image_integer_sample_counts =
                                     static_cast<GPU::Limits::Sample_count>(limits_a.sampledImageIntegerSampleCounts),
                                 .sampled_image_depth_sample_counts =
                                     static_cast<GPU::Limits::Sample_count>(limits_a.sampledImageDepthSampleCounts),
                                 .sampled_image_stencil_sample_counts =
                                     static_cast<GPU::Limits::Sample_count>(limits_a.sampledImageStencilSampleCounts),
                                 .storage_image_sample_counts = static_cast<GPU::Limits::Sample_count>(limits_a.storageImageSampleCounts),
                                 .max_sample_mask_words = limits_a.maxSampleMaskWords,
                                 .timestamp_compute_and_graphics = 1u == limits_a.timestampComputeAndGraphics,
                                 .timestamp_period = limits_a.timestampPeriod,
                                 .max_clip_distances = limits_a.maxClipDistances,
                                 .max_cull_distances = limits_a.maxCullDistances,
                                 .max_combined_clip_and_cull_distances = limits_a.maxCombinedClipAndCullDistances,
                                 .discrete_queue_priorities = limits_a.discreteQueuePriorities,
                                 .point_size_range = { limits_a.pointSizeRange[0], limits_a.pointSizeRange[1] },
                                 .line_width_hrange = { limits_a.lineWidthRange[0], limits_a.lineWidthRange[1] },
                                 .point_size_granularity = limits_a.pointSizeGranularity,
                                 .line_width_granularity = limits_a.lineWidthGranularity,
                                 .strict_lines = 1u == limits_a.strictLines,
                                 .standard_sample_locations = 1u == limits_a.standardSampleLocations,
                                 .optimal_buffer_copy_offset_alignment = limits_a.optimalBufferCopyOffsetAlignment,
                                 .optimal_buffer_copy_row_pitch_alignment = limits_a.optimalBufferCopyRowPitchAlignment,
                                 .non_coherent_atom_size = limits_a.nonCoherentAtomSize };
                    };
                    auto from_VkQueueFamilyProperties =
                        [](std::span<VkQueueFamilyProperties> queue_families_a) -> Vector<GPU::QueueFamily> {
                        Vector<GPU::QueueFamily> ret(queue_families_a.size());
                        std::size_t index = 0u;
                        for (const VkQueueFamilyProperties& vk_queue_family_properties : queue_families_a)
                        {
                            GPU::QueueFamily q { .kind = static_cast<GPU::QueueFamily::Kind>(vk_queue_family_properties.queueFlags),
                                                 .members = vk_queue_family_properties.queueCount,
                                                 .index = index++ };

                            log_inf("Q: kind: {}. members: {}, index: {}", static_cast<std::uint32_t>(q.kind), q.members, q.index);

                            ret.push_back(q);
                        }

                        return ret;
                    };

                    std::uint32_t gpus_count = 0;
                    vkEnumeratePhysicalDevices(vk_instance, &gpus_count, nullptr);
                    std::unique_ptr<VkPhysicalDevice[]> gpus_buffer = std::make_unique<VkPhysicalDevice[]>(gpus_count);
                    vkEnumeratePhysicalDevices(vk_instance, &gpus_count, gpus_buffer.get());

                    gpus.reserve(gpus_count);

                    for (std::uint32_t gpu_index = 0; gpu_index < gpus_count; gpu_index++)
                    {
                        VkPhysicalDeviceProperties vk_device_properties;
                        VkPhysicalDeviceFeatures vk_device_features;

                        vkGetPhysicalDeviceProperties(gpus_buffer[gpu_index], &vk_device_properties);
                        vkGetPhysicalDeviceFeatures(gpus_buffer[gpu_index], &vk_device_features);

                        std::uint32_t device_extensions_count = 0u;
                        vkEnumerateDeviceExtensionProperties(gpus_buffer[gpu_index], nullptr, &device_extensions_count, nullptr);

                        std::unique_ptr<VkExtensionProperties[]> vk_device_extensions_buffer;
                        Vector<std::string_view> device_extension_names(device_extensions_count);

                        if (0u != device_extensions_count)
                        {
                            vk_device_extensions_buffer = std::make_unique<VkExtensionProperties[]>(device_extensions_count);
                            vkEnumerateDeviceExtensionProperties(
                                gpus_buffer[gpu_index], nullptr, &device_extensions_count, vk_device_extensions_buffer.get());

                            for (std::uint32_t ex_index = 0; ex_index < device_extensions_count; ex_index++)
                            {
                                device_extension_names.emplace_back(vk_device_extensions_buffer[ex_index].extensionName);
                            }
                        }

                        bool is_primary = vk_device_properties.deviceName == primary_display_device_name;

                        std::uint32_t queue_family_property_count = 0u;
                        vkGetPhysicalDeviceQueueFamilyProperties(gpus_buffer[gpu_index], &queue_family_property_count, nullptr);

                        std::unique_ptr<VkQueueFamilyProperties[]> vk_queue_family_properties_buffer =
                            std::make_unique<VkQueueFamilyProperties[]>(queue_family_property_count);
                        vkGetPhysicalDeviceQueueFamilyProperties(
                            gpus_buffer[gpu_index], &queue_family_property_count, vk_queue_family_properties_buffer.get());

                        VkPhysicalDeviceProperties vk_physical_device_properties;
                        VkPhysicalDeviceFeatures vk_physical_device_features;

                        vkGetPhysicalDeviceProperties(gpus_buffer[gpu_index], &vk_physical_device_properties);
                        vkGetPhysicalDeviceFeatures(gpus_buffer[gpu_index], &vk_physical_device_features);

                        gpus.emplace_back(
                            gpus_buffer[gpu_index],
                            from_VkPhysicalDeviceType(vk_physical_device_properties.deviceType) |
                                (true == is_primary ? GPU::Kind::primary : GPU::Kind { 0x0u }),
                            from_VkPhysicalDeviceFeatures(vk_physical_device_features),
                            from_VkPhysicalDeviceLimits(vk_physical_device_properties.limits),
                            vk_device_properties.deviceName,
                            from_VkQueueFamilyProperties({ vk_queue_family_properties_buffer.get(), queue_family_property_count }),
                            device_extension_names);
                    }
                }
            }

            displays.shrink_to_fit();
            gpus.shrink_to_fit();

            lx::gpu::Context graphics_context;
            std::int32_t entry_point_ret = lx::app::entry_point(displays, gpus, graphics_context, windower, cmd_line);

            if (true == config.vulkan.enable_validation)
            {
                vkDestroyDebugUtilsMessengerEXT(vk_instance, vk_debug_messenger, nullptr);
            }

            lxlDestroy();

            if (nullptr != p_log_file)
            {
                fclose(p_log_file);
            }

            if (true == log_console_output)
            {
                fprintf(stdout, "Press any key to exit...\n");
                std::ignore = getc(stdin);
                FreeConsole();
            }

            return entry_point_ret;
        }
    }

    return -1;
}