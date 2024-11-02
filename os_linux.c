#include <sys/mman.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <stdbool.h>
#include <stdint.h>
#include <X11/keysym.h>
#include <time.h>
#include <dlfcn.h>

read_only OS_Key key_table[] = 
{
	[XK_F1] = OS_Key_F1, 
	[XK_F2] = OS_Key_F2, 
	[XK_F3] = OS_Key_F3, 
	[XK_F4] = OS_Key_F4, 
	[XK_F5] = OS_Key_F5, 
	[XK_F6] = OS_Key_F6, 
	[XK_F7] = OS_Key_F7,
	[XK_F8] = OS_Key_F8, 
	[XK_F9] = OS_Key_F9, 
	[XK_F10] = OS_Key_F10,
	[XK_F11] = OS_Key_F11, 
	[XK_F12] = OS_Key_F12,
	
	['a'] = OS_Key_A,
	['b'] = OS_Key_B,
	['c'] = OS_Key_C,
	['d'] = OS_Key_D,
	['e'] = OS_Key_E,
	['f'] = OS_Key_F,
	['g'] = OS_Key_G,
	['h'] = OS_Key_H,
	['i'] = OS_Key_I,
	['j'] = OS_Key_J,
	['k'] = OS_Key_K,
	['l'] = OS_Key_L,
	['m'] = OS_Key_M,
	['n'] = OS_Key_N,
	['o'] = OS_Key_O,
	['p'] = OS_Key_P,
	['q'] = OS_Key_Q,
	['r'] = OS_Key_R,
	['s'] = OS_Key_S,
	['t'] = OS_Key_T,
	['u'] = OS_Key_U,
	['v'] = OS_Key_V,
	['w'] = OS_Key_W,
	['x'] = OS_Key_X,
	['y'] = OS_Key_Y,
	['z'] = OS_Key_Z,
	
	[XK_0] = OS_Key_0,
	[XK_1] = OS_Key_1,
	[XK_2] = OS_Key_2,
	[XK_3] = OS_Key_3,
	[XK_4] = OS_Key_4,
	[XK_5] = OS_Key_5,
	[XK_6] = OS_Key_6,
	[XK_7] = OS_Key_7,
	[XK_8] = OS_Key_8,
	[XK_9] = OS_Key_9,
	
	[XK_Control_L] = OS_Key_LCTRL,
	[XK_Control_R] = OS_Key_RCTRL,
	[XK_Shift_L] = OS_Key_LSHIFT,
	[XK_Shift_R] = OS_Key_RSHIFT,
	[XK_Alt_L] = OS_Key_LALT,
	[XK_Alt_R] = OS_Key_RALT,
	
	[XK_Left] = OS_Key_LEFT,
	[XK_Right] = OS_Key_RIGHT,
	[XK_Up] = OS_Key_UP,
	[XK_Down] = OS_Key_DOWN,
	
	[XK_Tab] = OS_Key_TAB,
	[XK_Caps_Lock] = OS_Key_CAPS,
	[XK_Return] = OS_Key_ENTER,
	[XK_Escape] = OS_Key_ESC,
	[XK_space] = OS_Key_SPACE,
};

read_only OS_Key mouse_button_table[] = 
{
	[Button1] = OS_Key_LMB,
	[Button2] = OS_Key_MMB,
	[Button3] = OS_Key_RMB,
};

typedef struct OS_Window OS_Window;
struct OS_Window
{
	Window window;
	Atom del_window;
};

typedef struct OS_State OS_State;
struct OS_State
{
	Arena *arena;
	OS_Window win[OS_MAX_WIN];
	Display *display;
	int screen;
	s32 num;
};

pub OS_State *os_state;
pub OS_EventList event_list;
pub Arena *event_arena;

fn void os_innit()
{
	Arena *arena = arenaAlloc();
	os_state = push_struct(arena, OS_State);
	os_state->arena = arena;
	os_state->display = XOpenDisplay(0);
	os_state->screen = DefaultScreen(os_state->display);
	XAutoRepeatOff(os_state->display);
}

fn OS_Window *os_windowFromHandle(OS_Handle handle)
{
	return (OS_Window*)handle.u64[0];
}

fn OS_EventList os_pollEvents(Arena *arena)
{
	event_arena = arena;
	event_list = (OS_EventList){0};
	
	while (XPending(os_state->display))
	{
		XEvent event;
		XNextEvent(os_state->display, &event);
		
		switch(event.type)
		{
			case ClientMessage:
			{
				if ((Atom)event.xclient.data.l[0] == os_state->win[0].del_window)
				{
					OS_Event *os_event = os_pushEvent(event_arena, &event_list);
					os_event->kind = OS_EventKind_CloseRequested;
				}
			} break;
			
			case KeyPress:
			case KeyRelease:
			{
				b32 isDown = event.type == KeyPress;
				
				KeySym sym = XLookupKeysym(&event.xkey, 0);
				
				OS_Key key = key_table[sym];
				if(key)
				{
					OS_Event *os_event = os_pushEvent(event_arena, &event_list);
					
					os_event->key = key;
					os_event->kind = isDown ? OS_EventKind_Pressed : OS_EventKind_Released;
				}
				
			}break;
			
			case ButtonPress:
			case ButtonRelease:
			{
				OS_Event *os_event = os_pushEvent(event_arena, &event_list);
				
				os_event->key = mouse_button_table[event.xbutton.button];
				os_event->kind = ButtonRelease ? OS_EventKind_Released : OS_EventKind_Pressed;
			}break;
			
			case MotionNotify:
			{
    OS_Event *os_event = os_pushEvent(event_arena, &event_list);
    os_event->kind = OS_EventKind_MouseMove;
    os_event->mpos.x = event.xmotion.x;
    os_event->mpos.y = event.xmotion.y;
			} break;
			
		}
	}
	
	return event_list;
}

fn OS_Handle os_openWindow(char * title, f32 x, f32 y, f32 w, f32 h)
{
	OS_Window *win = os_state->win + os_state->num++;
	
	win->window = XCreateSimpleWindow(os_state->display, RootWindow(os_state->display, os_state->screen),
																																			x, y, w, h, 1, BlackPixel(os_state->display, os_state->screen),
																																			WhitePixel(os_state->display, os_state->screen));
	
	win->del_window = XInternAtom(os_state->display, "WM_DELETE_WINDOW", 0);
	XSetWMProtocols(os_state->display, win->window, &win->del_window, 1);
	XSelectInput(os_state->display, win->window, ExposureMask | KeyPressMask | KeyReleaseMask | ButtonReleaseMask | ButtonPressMask | PointerMotionMask | StructureNotifyMask);
	XMapWindow(os_state->display, win->window);
	
	OS_Handle out = {0};
	out.u64[0] = win;
	return out;
}

// TODO(mizu): We will eventually haver layers for other combos
// xlib/Vulkan layer ======================================================

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
	VkXlibSurfaceCreateInfoKHR xlib_surf_info = {
		.sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR,
		.pNext = 0,
		.flags = 0,
		.dpy = os_state->display,
		.window = os_windowFromHandle(handle)->window
	};
	
	VkResult res = vkCreateXlibSurfaceKHR(instance, &xlib_surf_info, 0, surface);
	return res;
}