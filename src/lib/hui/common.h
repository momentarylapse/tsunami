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


#include "../base/base.h"
#include <functional>

// which api?

/*#ifdef OS_WINDOWS
	#ifdef HUI_USE_GTK_ON_WINDOWS
		#define HUI_API_GTK
	#else
		#define HUI_API_WIN
	#endif
#else*/
	#define HUI_API_GTK
//#endif



#ifdef OS_WINDOWS
	#ifndef _WIN32_WINDOWS
		#ifndef _WIN32_WINDOWS
			#define _WIN32_WINDOWS 0x500
		#endif
		#ifndef _WIN32_WINNT
			#define _WIN32_WINNT 0x0500
		#endif
	#endif
#endif
#ifdef HUI_API_GTK
	#include <gtk/gtk.h>
	#include <gdk/gdkkeysyms.h>
#endif
#if defined(OS_LINUX) || defined(OS_MAC)
	#define _cdecl
#endif



typedef struct _XDisplay Display; // Xorg

namespace hui
{



#ifdef HUI_API_WIN
#else
	extern Display *x_display;
#endif




enum {
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
	CONTROL_MENU_BUTTON,
	CONTROL_HEADER_BAR,
	CONTROL_BASIC_WINDOW_LAYOUT,

	TOOL_ITEM_BUTTON,
	TOOL_ITEM_TOGGLEBUTTON,
	TOOL_ITEM_SEPARATOR,
	TOOL_ITEM_MENUBUTTON,

	MENU_ITEM_BUTTON,
	MENU_ITEM_TOGGLE,
	MENU_ITEM_SUBMENU,
	MENU_ITEM_SEPARATOR,
};

#ifdef HUGE
#undef HUGE
#endif

enum IconSize {
	SMALL,
	REGULAR,
	LARGE,
	HUGE,
	TOOLBAR_SMALL,
	TOOLBAR_LARGE
};

}

#endif
