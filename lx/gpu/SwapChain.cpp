// this
#include <lx/gpu/SwapChain.hpp>

// lx
#include <lx/utils/logger.hpp>

namespace lx::gpu {
SwapChain::SwapChain(VkDevice vk_device_a, VkSurfaceKHR vk_surface_a, const Properties& properties_a)
{
    VkSwapchainCreateInfoKHR vk_swap_chain_create_info { .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
                                                         .pNext = nullptr,
                                                         .flags = 0x0u,
                                                         .surface = vk_surface_a,
                                                         .minImageCount = static_cast<std::uint32_t>(properties_a.images_count),
                                                         .imageFormat = static_cast<VkFormat>(properties_a.format),
                                                         .imageColorSpace = static_cast<VkColorSpaceKHR>(properties_a.color_space),
                                                         .imageExtent = { .width = static_cast<std::uint32_t>(properties_a.extent.w),
                                                                          .height = static_cast<std::uint32_t>(properties_a.extent.h) },
                                                         .imageArrayLayers = 1u,
                                                         .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                                                         .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
                                                         .queueFamilyIndexCount = 0u,
                                                         .pQueueFamilyIndices = nullptr,
                                                         .preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR,
                                                         .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
                                                         .presentMode = static_cast<VkPresentModeKHR>(properties_a.mode),
                                                         .clipped = VK_TRUE,
                                                         .oldSwapchain = VK_NULL_HANDLE };
    auto res = vkCreateSwapchainKHR(vk_device_a, &vk_swap_chain_create_info, nullptr, &(this->vk_swap_chain));
    bool success = VK_SUCCESS == res;

    if (true == success)
    {
        std::uint32_t swap_chain_images_count = 0;

        vkGetSwapchainImagesKHR(vk_device_a, this->vk_swap_chain, &swap_chain_images_count, nullptr);
        this->vk_swap_chain_images.resize(swap_chain_images_count);
        vkGetSwapchainImagesKHR(vk_device_a, this->vk_swap_chain, &swap_chain_images_count, this->vk_swap_chain_images.get_buffer());

        this->vk_swap_chain_image_views.resize(swap_chain_images_count);
        for (std::uint32_t i = 0; i < swap_chain_images_count && true == success; i++)
        {
            VkImageViewCreateInfo vk_image_view_create_info {
                .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
                .pNext = nullptr,
                .flags = 0u,
                .image = this->vk_swap_chain_images[i],
                .viewType = VK_IMAGE_VIEW_TYPE_2D,
                .format = static_cast<VkFormat>(properties_a.format),
                .components { .r = VK_COMPONENT_SWIZZLE_IDENTITY,
                              .g = VK_COMPONENT_SWIZZLE_IDENTITY,
                              .b = VK_COMPONENT_SWIZZLE_IDENTITY,
                              .a = VK_COMPONENT_SWIZZLE_IDENTITY },
                .subresourceRange {
                    .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .baseMipLevel = 0u, .levelCount = 1u, .baseArrayLayer = 0u, .layerCount = 1u }
            };

            success =
                VK_SUCCESS == vkCreateImageView(vk_device_a, &vk_image_view_create_info, nullptr, &(this->vk_swap_chain_image_views[i]));
        }
    }
    else
    {
        log_err("vkCreateSwapchainKHR failure: {}", static_cast<std::underlying_type_t<decltype(res)>>(res));
    }
}
} // namespace lx::gpu