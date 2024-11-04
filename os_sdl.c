#undef fn
#include <SDL2/SDL.h>
#define fn static

read_only OS_Key key_table[] = {
    [SDL_SCANCODE_F1] = OS_Key_F1,
    [SDL_SCANCODE_F2] = OS_Key_F2,
    [SDL_SCANCODE_F3] = OS_Key_F3,
    [SDL_SCANCODE_F4] = OS_Key_F4,
    [SDL_SCANCODE_F5] = OS_Key_F5,
    [SDL_SCANCODE_F6] = OS_Key_F6,
    [SDL_SCANCODE_F7] = OS_Key_F7,
    [SDL_SCANCODE_F8] = OS_Key_F8,
    [SDL_SCANCODE_F9] = OS_Key_F9,
    [SDL_SCANCODE_F10] = OS_Key_F10,
    [SDL_SCANCODE_F11] = OS_Key_F11,
    [SDL_SCANCODE_F12] = OS_Key_F12,
    
    [SDL_SCANCODE_A] = OS_Key_A,
    [SDL_SCANCODE_B] = OS_Key_B,
    [SDL_SCANCODE_C] = OS_Key_C,
    [SDL_SCANCODE_D] = OS_Key_D,
    [SDL_SCANCODE_E] = OS_Key_E,
    [SDL_SCANCODE_F] = OS_Key_F,
    [SDL_SCANCODE_G] = OS_Key_G,
    [SDL_SCANCODE_H] = OS_Key_H,
    [SDL_SCANCODE_I] = OS_Key_I,
    [SDL_SCANCODE_J] = OS_Key_J,
    [SDL_SCANCODE_K] = OS_Key_K,
    [SDL_SCANCODE_L] = OS_Key_L,
    [SDL_SCANCODE_M] = OS_Key_M,
    [SDL_SCANCODE_N] = OS_Key_N,
    [SDL_SCANCODE_O] = OS_Key_O,
    [SDL_SCANCODE_P] = OS_Key_P,
    [SDL_SCANCODE_Q] = OS_Key_Q,
    [SDL_SCANCODE_R] = OS_Key_R,
    [SDL_SCANCODE_S] = OS_Key_S,
    [SDL_SCANCODE_T] = OS_Key_T,
    [SDL_SCANCODE_U] = OS_Key_U,
    [SDL_SCANCODE_V] = OS_Key_V,
    [SDL_SCANCODE_W] = OS_Key_W,
    [SDL_SCANCODE_X] = OS_Key_X,
    [SDL_SCANCODE_Y] = OS_Key_Y,
    [SDL_SCANCODE_Z] = OS_Key_Z,
    
    [SDL_SCANCODE_0] = OS_Key_0,
    [SDL_SCANCODE_1] = OS_Key_1,
    [SDL_SCANCODE_2] = OS_Key_2,
    [SDL_SCANCODE_3] = OS_Key_3,
    [SDL_SCANCODE_4] = OS_Key_4,
    [SDL_SCANCODE_5] = OS_Key_5,
    [SDL_SCANCODE_6] = OS_Key_6,
    [SDL_SCANCODE_7] = OS_Key_7,
    [SDL_SCANCODE_8] = OS_Key_8,
    [SDL_SCANCODE_9] = OS_Key_9,
    
    [SDL_SCANCODE_LCTRL] = OS_Key_LCTRL,
    [SDL_SCANCODE_RCTRL] = OS_Key_RCTRL,
    [SDL_SCANCODE_LSHIFT] = OS_Key_LSHIFT,
    [SDL_SCANCODE_RSHIFT] = OS_Key_RSHIFT,
    [SDL_SCANCODE_LALT] = OS_Key_LALT,
    [SDL_SCANCODE_RALT] = OS_Key_RALT,
    
    [SDL_SCANCODE_LEFT] = OS_Key_LEFT,
    [SDL_SCANCODE_RIGHT] = OS_Key_RIGHT,
    [SDL_SCANCODE_UP] = OS_Key_UP,
    [SDL_SCANCODE_DOWN] = OS_Key_DOWN,
    
    [SDL_SCANCODE_TAB] = OS_Key_TAB,
    [SDL_SCANCODE_CAPSLOCK] = OS_Key_CAPS,
    [SDL_SCANCODE_RETURN] = OS_Key_ENTER,
    [SDL_SCANCODE_ESCAPE] = OS_Key_ESC,
    [SDL_SCANCODE_SPACE] = OS_Key_SPACE,
};

read_only OS_Key mouse_button_table[] = 
{
    [SDL_BUTTON_LEFT]   = OS_Key_LMB,
    [SDL_BUTTON_MIDDLE] = OS_Key_MMB,
    [SDL_BUTTON_RIGHT]  = OS_Key_RMB,
};

typedef struct OS_Window OS_Window;
struct OS_Window
{
	SDL_Window *raw;
    OS_Key keys[OS_Key_COUNT];
};

typedef struct OS_State OS_State;
struct OS_State
{
	Arena *arena;
	OS_Window win[OS_MAX_WIN];
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
	
    SDL_Init(SDL_INIT_VIDEO);
}


fn OS_Window *os_windowFromHandle(OS_Handle handle)
{
	return (OS_Window*)handle.u64[0];
}

fn OS_EventList os_pollEvents(Arena *arena)
{
	event_arena = arena;
	event_list = (OS_EventList){0};
	
    SDL_Event event;
	
    while (SDL_PollEvent(&event))
	{
		switch(event.type)
		{
			case SDL_QUIT:
			{
                OS_Event *os_event = os_pushEvent(event_arena, &event_list);
                os_event->kind = OS_EventKind_CloseRequested;
			}break;
			
			case SDL_KEYDOWN:
            case SDL_KEYUP:
			{
				b32 isDown = event.type == SDL_KEYDOWN;
				SDL_Scancode sym = event.key.keysym.scancode;
                OS_Key key = key_table[sym];
                
                // TODO(mizu): too lazy for upper bound check
                if(key)
                {
                    if(isDown && os_state->win[0].keys[key] == 0)
                    {
                        OS_Event *os_event = os_pushEvent(event_arena, &event_list);
                        
                        os_event->key = key;
                        os_event->kind = OS_EventKind_Pressed;
                    }
                    else if(!isDown)
                    {
                        OS_Event *os_event = os_pushEvent(event_arena, &event_list);
                        
                        os_event->key = key;
                        os_event->kind = OS_EventKind_Released;
                    }
                    
                    os_state->win[0].keys[key] = isDown;
                }
                
            }break;
            
            case SDL_MOUSEBUTTONDOWN:
            case SDL_MOUSEBUTTONUP:
            {
                b32 isDown = event.type == SDL_MOUSEBUTTONDOWN;
                OS_Event *os_event = os_pushEvent(event_arena, &event_list);
                
                os_event->key = mouse_button_table[event.button.button];
                os_event->kind = isDown ? OS_EventKind_Pressed : OS_EventKind_Released;
            }break;
            
            case SDL_MOUSEMOTION:
            {
                OS_Event *os_event = os_pushEvent(event_arena, &event_list);
                os_event->kind = OS_EventKind_MouseMove;
                os_event->mpos.x = event.motion.x;
                os_event->mpos.y = event.motion.y;
            } break;
            
        }
    }
    
    return event_list;
}

fn OS_Handle os_openWindow(char * title, f32 x, f32 y, f32 w, f32 h)
{
    OS_Window *win = os_state->win + os_state->num++;
    
    win->raw = SDL_CreateWindow(title, x, y, w, h, SDL_WINDOW_SHOWN);
    
    OS_Handle out = {0};
    out.u64[0] = win;
    return out;
}