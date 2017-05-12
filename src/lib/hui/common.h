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


#include "../file/file.h"

#include <functional>


class Painter;
typedef struct _XDisplay Display; // Xorg

namespace hui
{



#ifdef HUI_API_WIN
#else
	extern Display *x_display;
#endif


class EventHandler : public VirtualBase
{
public:
};


typedef void kaba_callback();
typedef void kaba_member_callback(EventHandler *h);
typedef void kaba_member_callback_p(EventHandler *h, void *p);

typedef std::function<void()> Callback;
typedef std::function<void(Painter*)> CallbackP;



#define MAX_KEYBUFFER_DEPTH			128



// dialog controls (don't change the order!!!)
enum{
	CONTROL_BUTTON,
	CONTROL_DEFBUTTON,
	CONTROL_EDIT,
	CONTROL_LABEL,
	CONTROL_CHECKBOX,
	CONTROL_GROUP,
	CONTROL_COMBOBOX,
	CONTROL_TABCONTROL,
	CONTROL_LISTVIEW,
	CONTROL_TREEVIEW,
	CONTROL_ICONVIEW,
	CONTROL_PROGRESSBAR,
	CONTROL_IMAGE,
	CONTROL_COLORBUTTON,
	CONTROL_SLIDER,
	CONTROL_DRAWINGAREA,
	CONTROL_GRID,
	CONTROL_MULTILINEEDIT,
	CONTROL_SPINBUTTON,
	CONTROL_RADIOBUTTON,
	CONTROL_TOGGLEBUTTON,
	CONTROL_EXPANDER,
	CONTROL_SCROLLER,
	CONTROL_SEPARATOR,
	CONTROL_PANED,
	CONTROL_REVEALER,

	TOOL_ITEM_BUTTON,
	TOOL_ITEM_TOGGLEBUTTON,
	TOOL_ITEM_SEPARATOR,
	TOOL_ITEM_MENUBUTTON,

	MENU_ITEM_BUTTON,
	MENU_ITEM_TOGGLE,
	MENU_ITEM_SUBMENU,
	MENU_ITEM_SEPARATOR,
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

	NUM_KEYS,

	KEY_ANY,
	KEY_CONTROL = 256,
	KEY_SHIFT = 512,
	KEY_ALT = 1024
};

};

#endif
