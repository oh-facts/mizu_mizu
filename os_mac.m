#include <Cocoa/Cocoa.h>
#include <Foundation/Foundation.h>
#include <Carbon/Carbon.h>

read_only OS_Key key_table[] = 
{
	[kVK_F1] = OS_Key_F1, 
	[kVK_F2] = OS_Key_F2, 
	[kVK_F3] = OS_Key_F3, 
	[kVK_F4] = OS_Key_F4, 
	[kVK_F5] = OS_Key_F5, 
	[kVK_F6] = OS_Key_F6, 
	[kVK_F7] = OS_Key_F7,
	[kVK_F8] = OS_Key_F8, 
	[kVK_F9] = OS_Key_F9, 
	[kVK_F10] = OS_Key_F10,
	[kVK_F11] = OS_Key_F11, 
	[kVK_F12] = OS_Key_F12,
	
	[kVK_ANSI_A] = OS_Key_A,
	[kVK_ANSI_B] = OS_Key_B,
	[kVK_ANSI_C] = OS_Key_C,
	[kVK_ANSI_D] = OS_Key_D,
	[kVK_ANSI_E] = OS_Key_E,
	[kVK_ANSI_F] = OS_Key_F,
	[kVK_ANSI_G] = OS_Key_G,
	[kVK_ANSI_H] = OS_Key_H,
	[kVK_ANSI_I] = OS_Key_I,
	[kVK_ANSI_J] = OS_Key_J,
	[kVK_ANSI_K] = OS_Key_K,
	[kVK_ANSI_L] = OS_Key_L,
	[kVK_ANSI_M] = OS_Key_M,
	[kVK_ANSI_N] = OS_Key_N,
	[kVK_ANSI_O] = OS_Key_O,
	[kVK_ANSI_P] = OS_Key_P,
	[kVK_ANSI_Q] = OS_Key_Q,
	[kVK_ANSI_R] = OS_Key_R,
	[kVK_ANSI_S] = OS_Key_S,
	[kVK_ANSI_T] = OS_Key_T,
	[kVK_ANSI_U] = OS_Key_U,
	[kVK_ANSI_V] = OS_Key_V,
	[kVK_ANSI_W] = OS_Key_W,
	[kVK_ANSI_X] = OS_Key_X,
	[kVK_ANSI_Y] = OS_Key_Y,
	[kVK_ANSI_Z] = OS_Key_Z,
	
	[kVK_ANSI_0] = OS_Key_0,
	[kVK_ANSI_1] = OS_Key_1,
	[kVK_ANSI_2] = OS_Key_2,
	[kVK_ANSI_3] = OS_Key_3,
	[kVK_ANSI_4] = OS_Key_4,
	[kVK_ANSI_5] = OS_Key_5,
	[kVK_ANSI_6] = OS_Key_6,
	[kVK_ANSI_7] = OS_Key_7,
	[kVK_ANSI_8] = OS_Key_8,
	[kVK_ANSI_9] = OS_Key_9,
	
	[kVK_Control] = OS_Key_LCTRL,
	[kVK_RightControl] = OS_Key_RCTRL,
	[kVK_Shift] = OS_Key_LSHIFT,
	[kVK_RightShift] = OS_Key_RSHIFT,
	[kVK_Option] = OS_Key_LALT,
	[kVK_RightOption] = OS_Key_RALT,
	
	[kVK_LeftArrow] = OS_Key_LEFT,
	[kVK_RightArrow] = OS_Key_RIGHT,
	[kVK_UpArrow] = OS_Key_UP,
	[kVK_DownArrow] = OS_Key_DOWN,
	
	[kVK_Tab] = OS_Key_TAB,
	[kVK_CapsLock] = OS_Key_CAPS,
	[kVK_Return] = OS_Key_ENTER,
	[kVK_Escape] = OS_Key_ESC,
	[kVK_Space] = OS_Key_SPACE,
};

pub OS_EventList event_list;
pub Arena *event_arena;

@interface windowDelegate: NSObject <NSWindowDelegate>;
@end

@implementation windowDelegate

- (void)windowWillClose:(NSNotification*)notification {
	OS_Event *os_event = os_pushEvent(event_arena, &event_list);
	os_event->kind = OS_EventKind_CloseRequested;
}
@end

typedef struct OS_Window OS_Window;
struct OS_Window
{
	NSWindow *raw;
	windowDelegate *del;
};

typedef struct OS_State OS_State;
struct OS_State
{
	Arena *arena;
	OS_Window win[OS_MAX_WIN];
	s32 num;
};

pub OS_State *os_state;

fn void os_innit()
{
	Arena *arena = arenaAlloc();
	os_state = push_struct(arena, OS_State);
	os_state->arena = arena;
}

fn OS_EventList os_pollEvents(Arena *arena)
{
	event_arena = arena;
	event_list = (OS_EventList){0};
	
	NSEvent *event;
	do {
		event = [NSApp nextEventMatchingMask: NSEventMaskAny
											untilDate: nil
											inMode: NSDefaultRunLoopMode
											dequeue: YES];
		
		switch([event type])
		{
			case NSEventTypeKeyDown:
			case NSEventTypeKeyUp:
			{
				b32 isDown = [event type] == NSEventTypeKeyDown;
				OS_Key key = key_table[[event keyCode]];
				
				if (key)
				{
					OS_Event *os_event = os_pushEvent(event_arena, &event_list);
					os_event->key = key;
					os_event->kind = isDown ? OS_EventKind_Pressed : OS_EventKind_Released;
				}
			} break;
			
			case NSEventTypeMouseMoved:
			{
				OS_Event *os_event = os_pushEvent(event_arena, &event_list);
				os_event->kind = OS_EventKind_MouseMove;
				os_event->mpos.x = [event locationInWindow].x;
				os_event->mpos.y = [event locationInWindow].y;
			} break;
			
			case NSEventTypeLeftMouseDown:
			{
				OS_Event *os_event = os_pushEvent(event_arena, &event_list);
				os_event->key = OS_Key_LMB;
				os_event->kind = OS_EventKind_Pressed;
			} break;
			
			case NSEventTypeLeftMouseUp:
			{
				OS_Event *os_event = os_pushEvent(event_arena, &event_list);
				os_event->key = OS_Key_LMB;
				os_event->kind = OS_EventKind_Released;
			} break;
			
			case NSEventTypeRightMouseDown:
			{
				OS_Event *os_event = os_pushEvent(event_arena, &event_list);
				os_event->key = OS_Key_RMB;
				os_event->kind = OS_EventKind_Pressed;
			} break;
			
			case NSEventTypeRightMouseUp:
			{
				OS_Event *os_event = os_pushEvent(event_arena, &event_list);
				os_event->key = OS_Key_RMB;
				os_event->kind = OS_EventKind_Released;
			} break;
			
			case NSEventTypeOtherMouseDown:
			{
				OS_Event *os_event = os_pushEvent(event_arena, &event_list);
				os_event->key = OS_Key_MMB;
				os_event->kind = OS_EventKind_Pressed;
			} break;
			
			case NSEventTypeOtherMouseUp:
			{
				OS_Event *os_event = os_pushEvent(event_arena, &event_list);
				os_event->key = OS_Key_MMB;
				os_event->kind = OS_EventKind_Released;
			} break;
			
			default:
			{
				[NSApp sendEvent: event];
			}
		}
		
	}while(event != nil);
	
	return event_list;
}

fn OS_Handle os_openWindow(char * title, f32 x, f32 y, f32 w, f32 h)
{
	OS_Window *win = os_state->win + os_state->num++;
	
	NSRect win_rect = NSMakeRect(x, y, w, h);
	win->raw = [[NSWindow alloc] initWithContentRect: win_rect 
													styleMask: NSWindowStyleMaskTitled | NSWindowStyleMaskClosable |
													NSWindowStyleMaskMiniaturizable | NSWindowStyleMaskResizable
													
													backing: NSBackingStoreBuffered
													defer: NO];
	
	[win->raw setTitle: [NSString stringWithUTF8String:title]];
	[win->raw makeKeyAndOrderFront: nil];
	
	windowDelegate *del = [[windowDelegate alloc]init];
	win->del = del;
	[win->raw setDelegate: del];
	
	OS_Handle out = {0};
	out.u64[0] = win;
	return out;
}

#include <vulkan/vulkan_macos.h>

PFN_vkCreateMacOSSurfaceMVK vkCreateMacOSSurfaceMVK;

// NOTE(mizu): Untested!
fn OS_Handle os_vulkan_loadLibrary()
{
	return os_loadLibrary("libvulkan.dylib");
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
	VkMacOSSurfaceCreateInfoMVK macos_surf_info = {
		.sType = VK_STRUCTURE_TYPE_MACOS_SURFACE_CREATE_INFO_MVK,
		.pNext = 0,
		.flags = 0,
		.dpy = os_state->display,
		.window = os_windowFromHandle(handle)->window
	};
	
	VkResult res = vkCreateXlibSurfaceKHR(r_vulkan_state->instance, &xlib_surf_info, NULL, &r_vulkan_state->surface);
	return res;
}