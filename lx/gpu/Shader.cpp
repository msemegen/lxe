// this
#include <lx/gpu/Shader.hpp>

// lx
#include <lx/utils/logger.hpp>

// std
#include <cassert>

namespace lx::gpu {
Shader::Shader(VkDevice vk_device_a, const lx::containers::Vector<std::byte>& data_a, const Properties& properties_a)
    : vk_device(vk_device_a)
    , properties(properties_a)
{
    assert(false == data_a.is_empty());

    VkShaderModuleCreateInfo vk_shader_module_create_info { .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
                                                            .pNext = nullptr,
                                                            .flags = 0x0u,
                                                            .codeSize = data_a.get_length(),
                                                            .pCode = reinterpret_cast<const std::uint32_t*>(data_a.get_buffer()) };

    auto res = vkCreateShaderModule(vk_device_a, &vk_shader_module_create_info, nullptr, &(this->vk_shader_module));

    if (VK_SUCCESS != res || VK_NULL_HANDLE == this->vk_shader_module)
    {
        log_err("vkCreateShaderModule failed: {}", static_cast<std::underlying_type_t<decltype(res)>>(res));
    }
}
void Shader::destroy()
{
    vkDestroyShaderModule(this->vk_device, this->vk_shader_module, nullptr);
}
} // namespace lx::gpu