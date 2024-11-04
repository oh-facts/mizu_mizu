#include <SDL2/SDL_syswm.h>
#include <X11/Xlib.h>
#define VK_USE_PLATFORM_XLIB_KHR
#include "vulkan/vulkan_xlib.h"

PFN_vkCreateXlibSurfaceKHR vkCreateXlibSurfaceKHR;

fn OS_Handle os_vulkan_loadLibrary()
{
    return os_loadLibrary("libvulkan.so.1");
}

fn void os_vulkan_loadSurfaceFunction(OS_Handle vkdll)
{
    vkCreateXlibSurfaceKHR = (PFN_vkCreateXlibSurfaceKHR)os_loadFunction(vkdll, "vkCreateXlibSurfaceKHR");
}

fn char *os_vulkan_surfaceExtentionName()
{
    return VK_KHR_XLIB_SURFACE_EXTENSION_NAME;
}

fn VkResult os_vulkan_createSurface(OS_Handle handle, VkInstance instance, VkSurfaceKHR *surface)
{
    SDL_SysWMinfo wmInfo;
    SDL_VERSION(&wmInfo.version);
    SDL_GetWindowWMInfo(os_state->win[0].raw, &wmInfo);
    
    Display *display = wmInfo.info.x11.display;
    Window x11_window = wmInfo.info.x11.window;
    
    VkXlibSurfaceCreateInfoKHR xlib_surf_info = {
        .sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR,
        .pNext = 0,
        .flags = 0,
        .dpy = display,
        .window = x11_window
    };
    
    VkResult res = vkCreateXlibSurfaceKHR(instance, &xlib_surf_info, 0, surface);
    return res;
}