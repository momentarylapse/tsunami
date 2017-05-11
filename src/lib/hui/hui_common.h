/*----------------------------------------------------------------------------*\
| Hui common                                                                   |
| -> configuration for hui library                                             |
|                                                                              |
| vital properties:                                                            |
|                                                                              |
| last update: 2009.12.05 (c) by MichiSoft TM                                  |
\*----------------------------------------------------------------------------*/

#ifndef _HUI_COMMON_EXISTS_
#define _HUI_COMMON_EXISTS_


#include "../config.h"
#include "../base/base.h"

// which api?

#ifdef OS_WINDOWS
	#ifdef HUI_USE_GTK_ON_WINDOWS
		#define HUI_API_GTK
	#else
		#define HUI_API_WIN
	#endif
#else
	#define HUI_API_GTK
#endif



#ifdef OS_WINDOWS
	#ifndef _WIN32_WINDOWS
		#ifndef _WIN32_WINDOWS
			#define _WIN32_WINDOWS 0x500
		#endif
		#ifndef _WIN32_WINNT
			#define _WIN32_WINNT 0x0500
		#endif
	#endif
	#include <windows.h>
#endif
#ifdef HUI_API_GTK
	#include <gtk/gtk.h>
	#include <gdk/gdkkeysyms.h>
#endif
#ifdef OS_LINUX
	#define _cdecl
#endif


#ifdef HUI_API_WIN
#else
	extern void *_hui_x_display_;
	#define hui_x_display (Display*)_hui_x_display_
#endif

#include "../file/file.h"

#include <functional>

class Painter;



class HuiEventHandler : public VirtualBase
{
public:
};


typedef void hui_kaba_callback();
typedef void hui_kaba_member_callback(HuiEventHandler *h);
typedef void hui_kaba_member_callback_p(HuiEventHandler *h, void *p);

typedef std::function<void()> HuiCallback;
typedef std::function<void(Painter*)> HuiCallbackP;



#define HUI_MAX_KEYBUFFER_DEPTH			128



// dialog controls (don't change the order!!!)
enum{
	HUI_KIND_BUTTON,
	HUI_KIND_DEFBUTTON,
	HUI_KIND_EDIT,
	HUI_KIND_LABEL,
	HUI_KIND_CHECKBOX,
	HUI_KIND_GROUP,
	HUI_KIND_COMBOBOX,
	HUI_KIND_TABCONTROL,
	HUI_KIND_LISTVIEW,
	HUI_KIND_TREEVIEW,
	HUI_KIND_ICONVIEW,
	HUI_KIND_PROGRESSBAR,
	HUI_KIND_IMAGE,
	HUI_KIND_COLORBUTTON,
	HUI_KIND_SLIDER,
	HUI_KIND_DRAWINGAREA,
	HUI_KIND_GRID,
	HUI_KIND_MULTILINEEDIT,
	HUI_KIND_SPINBUTTON,
	HUI_KIND_RADIOBUTTON,
	HUI_KIND_TOGGLEBUTTON,
	HUI_KIND_EXPANDER,
	HUI_KIND_SCROLLER,
	HUI_KIND_SEPARATOR,
	HUI_KIND_PANED,
	HUI_KIND_REVEALER,

	HUI_KIND_TOOL_BUTTON,
	HUI_KIND_TOOL_TOGGLEBUTTON,
	HUI_KIND_TOOL_SEPARATOR,
	HUI_KIND_TOOL_MENUBUTTON,

	HUI_KIND_MENU_ITEM,
	HUI_KIND_MENU_TOGGLEITEM,
	HUI_KIND_MENU_SUBMENU,
	HUI_KIND_MENU_SEPARATOR,
};

// key codes
enum{
	KEY_LCONTROL,
	KEY_RCONTROL,
	KEY_LSHIFT,
	KEY_RSHIFT,
	KEY_LALT,
	KEY_RALT,
	KEY_ADD,
	KEY_SUBTRACT,
	KEY_FENCE,		// "Raute"???
	KEY_END,
	KEY_NEXT,
	KEY_PRIOR,
	KEY_UP,
	KEY_DOWN,
	KEY_LEFT,
	KEY_RIGHT,
	KEY_RETURN,
	KEY_ESCAPE,
	KEY_INSERT,
	KEY_DELETE,
	KEY_SPACE,
	KEY_F1,
	KEY_F2,
	KEY_F3,
	KEY_F4,
	KEY_F5,
	KEY_F6,
	KEY_F7,
	KEY_F8,
	KEY_F9,
	KEY_F10,
	KEY_F11,
	KEY_F12,
	KEY_0,
	KEY_1,
	KEY_2,
	KEY_3,
	KEY_4,
	KEY_5,
	KEY_6,
	KEY_7,
	KEY_8,
	KEY_9,
	KEY_A,
	KEY_B,
	KEY_C,
	KEY_D,
	KEY_E,
	KEY_F,
	KEY_G,
	KEY_H,
	KEY_I,
	KEY_J,
	KEY_K,
	KEY_L,
	KEY_M,
	KEY_N,
	KEY_O,
	KEY_P,
	KEY_Q,
	KEY_R,
	KEY_S,
	KEY_T,
	KEY_U,
	KEY_V,
	KEY_W,
	KEY_X,
	KEY_Y,
	KEY_Z,
	KEY_BACKSPACE,
	KEY_TAB,
	KEY_HOME,
	KEY_NUM_0,
	KEY_NUM_1,
	KEY_NUM_2,
	KEY_NUM_3,
	KEY_NUM_4,
	KEY_NUM_5,
	KEY_NUM_6,
	KEY_NUM_7,
	KEY_NUM_8,
	KEY_NUM_9,
	KEY_NUM_ADD,
	KEY_NUM_SUBTRACT,
	KEY_NUM_MULTIPLY,
	KEY_NUM_DIVIDE,
	KEY_NUM_COMMA,
	KEY_NUM_ENTER,
	KEY_COMMA,
	KEY_DOT,
	KEY_SMALLER,
	KEY_SZ,
	KEY_AE,
	KEY_OE,
	KEY_UE,
	KEY_GRAVE,
	KEY_LWINDOWS,
	KEY_RWINDOWS,

	HUI_NUM_KEYS,

	KEY_ANY,
	KEY_CONTROL = 256,
	KEY_SHIFT = 512,
	KEY_ALT = 1024
};

#endif
