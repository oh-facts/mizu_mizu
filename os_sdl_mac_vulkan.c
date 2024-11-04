#include <Cocoa/Cocoa.h>
#include <vulkan/vulkan_macos.h>
#include <SDL2/SDL_syswm.h>
#include <SDL2/SDL_vulkan.h>

PFN_vkCreateMacOSSurfaceMVK vkCreateMacOSSurfaceMVK;

// NOTE(mizu): Untested!
fn OS_Handle os_vulkan_loadLibrary()
{
	return os_loadLibrary("/usr/local/lib/libvulkan.1.dylib");
}

fn void os_vulkan_loadSurfaceFunction(OS_Handle vkdll)
{
	vkCreateMacOSSurfaceMVK = (PFN_vkCreateMacOSSurfaceMVK)os_loadFunction(vkdll, "vkCreateMacOSSurfaceMVK");
}

fn char *os_vulkan_surfaceExtentionName()
{
	return VK_MVK_macos_surface;
}

fn VkResult os_vulkan_createSurface(OS_Handle handle, VkInstance instance, VkSurfaceKHR *surface)
{
	SDL_SysWMinfo wmInfo;
    SDL_VERSION(&wmInfo.version);
    SDL_GetWindowWMInfo(os_state->win[0].raw, &wmInfo);
    
    NSView *nsView = wmInfo.info.cocoa.view;
    
    VkMacOSSurfaceCreateInfoMVK macos_surf_info = {
        .sType = VK_STRUCTURE_TYPE_MACOS_SURFACE_CREATE_INFO_MVK,
        .pNext = 0,
        .flags = 0,
        .pView = nsView
    };
    
    VkResult res = vkCreateMacOSSurfaceMVK(instance, &macos_surf_info, NULL, surface);
    return res;
}