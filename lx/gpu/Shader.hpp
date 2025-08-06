#pragma once

// lx
#include <lx/common/non_copyable.hpp>
#include <lx/containers/String.hpp>
#include <lx/containers/Vector.hpp>
#include <lx/gpu/Format.hpp>

// externals
#include <lxl/lxl.h>

// std
#include <cstdint>

namespace lx::gpu {
class Shader : private lx::common::non_copyable
{
public:
    enum class Kind : std::uint32_t
    {
        fragment = VK_SHADER_STAGE_FRAGMENT_BIT,
        vertex = VK_SHADER_STAGE_VERTEX_BIT
    };

    struct Properties
    {
        Kind kind;
        std::string_view entry_point_name;
    };

    using enum Kind;

    bool is_created() const
    {
        return VK_NULL_HANDLE != this->vk_shader_module;
    }

    operator VkShaderModule() const
    {
        return this->vk_shader_module;
    }

    const Properties& get_properties() const
    {
        return this->properties;
    }

private:
    Shader(VkDevice vk_device_a, const lx::containers::Vector<std::byte>& data_a, const Properties& properties_a);
    void destroy();

    VkShaderModule vk_shader_module = VK_NULL_HANDLE;
    VkDevice vk_device = VK_NULL_HANDLE;

    Properties properties;

    friend class Device;
};
} // namespace lx::gpu