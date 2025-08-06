#pragma once

// lx
#include <lx/common/non_copyable.hpp>

// lxl
#include <lxl/lxl.h>

namespace lx::gpu {
class RenderPass : private lx::common::non_copyable
{
public:
    struct SubPass
    {
    };

    operator VkRenderPass() const
    {
        return this->vk_render_pass;
    }

private:
    VkRenderPass vk_render_pass = VK_NULL_HANDLE;

    friend class Device;
};
} // namespace lx::gpu