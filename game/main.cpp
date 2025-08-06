#include <lx/app.hpp>
#include <lx/gpu/Device.hpp>
#include <lx/utils/logger.hpp>

// test
#include <lx/gpu/CommandList.hpp>

namespace {
lx::containers::Vector<std::byte> load_shader(std::string_view file_name_a)
{
    FILE* fp = nullptr;
    auto err = fopen_s(&fp, file_name_a.data(), "rb");

    if (0 == err)
    {
        fseek(fp, 0, SEEK_END);
        auto file_length = ftell(fp);
        fseek(fp, 0, SEEK_SET);

        if (-1 != file_length && file_length > 0 && 0 == ftell(fp))
        {
            lx::containers::Vector<std::byte> ret;
            ret.resize(file_length);

            fread_s(ret.get_buffer(), ret.get_capacity(), sizeof(std::byte), ret.get_capacity(), fp);
            fclose(fp);

            return ret;
        }
    }

    return {};
}
} // namespace

class App : public lx::Windower::Events::PositionChange::Callback
{
public:
    virtual bool on_position_change(std::uint64_t, std::uint64_t) override
    {
        printf("position change!!!\n");
        return true;
    }
};

void lx::app::setup(lx::common::out<lx::app::Config> config_a)
{
    // log config
    config_a->log.console = true;
    config_a->log.path = "log.txt";

    // app config
    config_a->app.name = "test game";

    // vulkan config
    config_a->vulkan.enable_validation = true;
}

std::int32_t lx::app::entry_point(std::span<const lx::devices::Display> displays_a,
                                  std::span<const lx::devices::GPU> gpus_a,
                                  gpu::Context& graphics_context_a,
                                  lx::Windower& windower_a,
                                  std::string_view cmd_line_a)
{
    using namespace lx::common;
    using namespace lx::gpu;
    using namespace lx::utils;

    App app;

    log_set_filter(logger::dbg | logger::inf | logger::err);

    log_inf("Command line args: \"{}\"", cmd_line_a);
    log_inf("Displays count: {}", displays_a.size());
    log_inf("GPUs count: {}", gpus_a.size());

    for (auto gpu : gpus_a)
    {
        log_inf("GPU: {}", gpu.name.get_cstring());
        for (const auto q : gpu.queue_families)
        {
            log_inf("\tQ: kind: {}. members: {}, index: {}", static_cast<std::uint32_t>(q.kind), q.members, q.index);
        }
    }

    {
        auto canvas1 = windower_a.create<canvas::framed>(
            displays_a[0], Canvas<canvas::framed>::Properties { .title = gpus_a[0].name, .extent { .w = 800u, .h = 600u } });

        auto canvas2 = windower_a.create<canvas::framed>(
            displays_a[0], Canvas<canvas::framed>::Properties { .title = gpus_a[0].name, .extent { .w = 800u, .h = 600u } });

        if (true == canvas1.is_created() && true == canvas2.is_created())
        {
            log_inf("Canvases created");
            auto gpu_device1 = graphics_context_a.create<Device>(
                gpus_a[0], canvas1, Device::Properties { .features = Device::Feature::none, .extensions {} });

            auto gpu_device2 = graphics_context_a.create<Device>(
                gpus_a[1], canvas2, Device::Properties { .features = Device::Feature::none, .extensions {} });

            if (true == gpu_device1.is_created() && true == gpu_device2.is_created())
            {
                auto swap_chain = gpu_device1.create<lx::gpu::SwapChain>(
                    lx::gpu::SwapChain::Properties { .format = SwapChain::Format::r8g8b8a8_srgb,
                                                     .color_space = SwapChain::ColorSpace::srgb_nonlinear_khr,
                                                     .mode = SwapChain::Mode::fifo,
                                                     .extent { canvas1.get_properties().extent },
                                                     .images_count = 2u });
                auto rendering_queue = gpu_device1.create<lx::gpu::Queue>(
                    lx::gpu::Queue::Properties { .kind = Queue::graphics | Queue::transfer, .presentation = true });
                auto command_pool = gpu_device1.create<lx::gpu::CommandPool>(rendering_queue);
                auto command_list = gpu_device1.create<lx::gpu::CommandList<command_list::graphics | command_list::transfer>>(command_pool);

                command_list.start();

                command_list.stop();
                rendering_queue.submit(command_list);

                std::array blend_attachments = { Pipeline<pipeline::graphics>::Blend::Attachment { .enable = false } };

                auto vertex_shader_data = load_shader("shaders/vertex.bin");
                auto fragment_shader_data = load_shader("shaders/fragment.bin");

                if (false == vertex_shader_data.is_empty() && false == fragment_shader_data.is_empty())
                {
                    std::array shaders = {
                        gpu_device1.create<Shader>(fragment_shader_data,
                                                   Shader::Properties { .kind = Shader::fragment, .entry_point_name = "main" }),
                        gpu_device1.create<Shader>(vertex_shader_data,
                                                   Shader::Properties { .kind = Shader::vertex, .entry_point_name = "main" })
                    };

                    std::array vertex_attributes = {
                        Pipeline<pipeline::graphics>::ShaderInput::Vertex::Attribute {
                            .kind = Pipeline<pipeline::graphics>::ShaderInput::Vertex::Attribute::r32g32_sfloat,
                            .location = 0u,
                            .padding_in_bytes = 0u },
                        Pipeline<pipeline::graphics>::ShaderInput::Vertex::Attribute {
                            .kind = Pipeline<pipeline::graphics>::ShaderInput::Vertex::Attribute::r32g32b32_sfloat,
                            .location = 1u,
                            .padding_in_bytes = 0u }
                    };

                    std::array vertex_layouts = { Pipeline<pipeline::graphics>::ShaderInput::Vertex {
                        .rate = Pipeline<pipeline::graphics>::ShaderInput::Vertex::Rate::vertex, .attributes = vertex_attributes } };

                    std::array set_0_bindings = { Pipeline<pipeline::graphics>::ShaderInput::GlobalData::Binding {
                        .kind = Pipeline<pipeline::graphics>::ShaderInput::GlobalData::Binding::uniform_buffer,
                        .shader = Pipeline<pipeline::graphics>::ShaderInput::GlobalData::Binding::Shader::vertex,
                        .count = 1u,
                        .size_in_bytes = sizeof(float) * 4u } };
                    std::array global_data_layouts { Pipeline<pipeline::graphics>::ShaderInput::GlobalData {
                        .bindings { set_0_bindings } } };

                    std::array clips = { Pipeline<pipeline::graphics>::Clip { .viewport {
                        .position { .x = 0, .y = 0 }, .size { .w = 800u, .h = 600u }, .min_depth = 0.0f, .max_depth = 1.0f } } };

                    auto graphics_pipeline = gpu_device1.create<Pipeline<pipeline::graphics>>(
                        Pipeline<pipeline::graphics>::Properties {
                            .primitive { .fill = Pipeline<pipeline::graphics>::Primitive::Fill::full,
                                         .cull = Pipeline<pipeline::graphics>::Primitive::Cull::back,
                                         .front_face = Pipeline<pipeline::graphics>::Primitive::Front::clockwise,
                                         .topology = Pipeline<pipeline::graphics>::Primitive::Topology::triangle_list },
                            .depth { .test = true, .write = false, .bias_constant_factor = 1.0f, .bias_slope_factor = 0.0f },
                            .stencil { .test = false },
                            .multisampling { .rasterization_samples = Pipeline<pipeline::graphics>::Multisampling::SampleCount::_1,
                                             .alpha_to_coverage = false,
                                             .sample_mask = 0x0u },
                            .blend { .attachments { blend_attachments }, .constants { 0.0f, 0.0f, 0.0f, 0.0f } },
                            .shader_input { .vertex_layouts { vertex_layouts }, .global_data_layouts { global_data_layouts } },
                            .shaders { shaders },
                            .clips { clips } },
                        RenderPass {});

                    if (not graphics_pipeline.is_created())
                    {
                        return -2;
                    }

                    // foo(static_cast<const lx::gpu::CommandList<command_list::graphics>&>(command_list));

                    auto transfer_list = static_cast<lx::gpu::CommandList<command_list::graphics>>(command_list);

                    // auto buffer = gpu_device1.create<lx::gpu::Buffer>(&transfer_list, lx::gpu::Buffer::Properties {});

                    // auto command_pool = gpu_device1.create<lx::gpu::CommandPool>(rendering_queue);
                    // auto command_buffer = gpu_device1.create<lx::gpu::CommandBuffer>(command_pool);

                    /*
                     *  .swap_chain {  }
                     */

                    // if (true == gpu_device1.is_created() /* && true == gpu_device2->is_created()*/)
                    //{
                    log_inf("GPU context created");

                    windower_a.set_visible(canvas1, true);
                    windower_a.set_visible(canvas2, true);

                    windower_a.events.position_change.register_callback(inout(canvas1), &app);

                    bool c1 = false;
                    bool c2 = false;

                    // auto p = gpu_device1->create<Graphics>(
                    //     { .primitive { .polygon_mode = Graphics::Primitive::PolygonMode::fill,
                    //                    .cull_mode = Graphics::Primitive::CullMode::front | Graphics::Primitive::CullMode::back,
                    //                    .front_face = Graphics::Primitive::FrontFace::clockwise,
                    //                    .topology = Graphics::Primitive::Topology::triangle_list },
                    //       .depth {},
                    //       .stencil {},
                    //       .multisampling {},
                    //       .blend {},
                    //       .vertex_input {},
                    //       .shaders {},
                    //       .clips {} });

                    do
                    {
                        c1 = windower_a.update(out(canvas1));
                        c2 = windower_a.update(out(canvas2));
                    } while (true == c1 || true == c2);
                }

                gpu_device1.destroy(out(command_list));
                gpu_device1.destroy(out(command_pool));
                gpu_device1.destroy(out(swap_chain));

                if (true == gpu_device1.is_created() /*&& true == gpu_device2->is_created()*/)
                {
                    log_inf("GPU context destroyed");
                }

                graphics_context_a.destroy(out(gpu_device1));
                graphics_context_a.destroy(out(gpu_device2));
            }
        }

        windower_a.destroy(out(canvas1));
        windower_a.destroy(out(canvas2));
    }

    log_inf("Window destroyed");

    return 0;
}
