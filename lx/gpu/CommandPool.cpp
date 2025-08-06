// this
#include <lx/gpu/CommandPool.hpp>

// lx
#include <lx/utils/logger.hpp>

namespace lx::gpu {
CommandPool::CommandPool(VkDevice vk_device_a, std::uint32_t queue_family_index_a, lx::gpu::Queue::Kind queue_kind_a)
    : vk_device(vk_device_a)
    , queue_kind(queue_kind_a)
{
    const VkCommandPoolCreateInfo vk_command_pool_create_info {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = queue_family_index_a,
    };

    auto res = vkCreateCommandPool(vk_device_a, &vk_command_pool_create_info, nullptr, &(this->vk_command_pool));

    if (VK_SUCCESS != res)
    {
        log_err("vkCreateCommandPool failed: {}", static_cast<std::underlying_type_t<decltype(res)>>(res));
    }
}
void CommandPool::destroy()
{
    vkDestroyCommandPool(this->vk_device, this->vk_command_pool, nullptr);
    this->vk_command_pool = VK_NULL_HANDLE;
}
} // namespace lx::gpu