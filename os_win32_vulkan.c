#define VK_USE_PLATFORM_WIN32_KHR
#include "vulkan/vulkan_win32.h"

PFN_vkCreateWin32SurfaceKHR vkCreateWin32SurfaceKHR;

fn OS_Handle os_vulkan_loadLibrary()
{
	return os_loadLibrary("vulkan-1.dll");
}

fn void os_vulkan_loadSurfaceFunction(OS_Handle vkdll)
{
	vkCreateWin32SurfaceKHR = (PFN_vkCreateWin32SurfaceKHR)os_loadFunction(vkdll, "vkCreateWin32SurfaceKHR");
}

fn char *os_vulkan_surfaceExtentionName()
{
	return VK_KHR_WIN32_SURFACE_EXTENSION_NAME;
}

fn VkResult os_vulkan_createSurface(OS_Handle handle, VkInstance instance, VkSurfaceKHR *surface)
{
	VkWin32SurfaceCreateInfoKHR win32_surf_info = {
		.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
		.pNext = 0,
		.flags = 0,
		.hinstance = GetModuleHandle(0),
		.hwnd = os_windowFromHandle(handle)->hwnd
	};
	
	VkResult res = vkCreateWin32SurfaceKHR(instance, &win32_surf_info, 0, surface);
	return res;
}