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
#include <functional>

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


typedef struct _XDisplay Display; // Xorg

namespace hui
{



#ifdef HUI_API_WIN
#else
	extern Display *x_display;
#endif




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
	CONTROL_MENU_BUTTON,

	TOOL_ITEM_BUTTON,
	TOOL_ITEM_TOGGLEBUTTON,
	TOOL_ITEM_SEPARATOR,
	TOOL_ITEM_MENUBUTTON,

	MENU_ITEM_BUTTON,
	MENU_ITEM_TOGGLE,
	MENU_ITEM_SUBMENU,
	MENU_ITEM_SEPARATOR,
};

}

#endif
