/*----------------------------------------------------------------------------*\
| CHui                                                                         |
| -> Heroic User Interface                                                     |
| -> abstraction layer for GUI                                                 |
|   -> Windows (default api) or Linux (Gtk+)                                   |
|                                                                              |
| vital properties:                                                            |
|                                                                              |
| last updated: 2009.12.05 (c) by MichiSoft TM                                 |
\*----------------------------------------------------------------------------*/

#ifndef _HUI_EXISTS_
#define _HUI_EXISTS_

#include "common.h"
#if __has_include("../image/image.h")
	#include "../image/image.h"
#else
	typedef int Image;
	typedef int color;
#endif

namespace hui
{

extern string Version;




class Window;
class Menu;
class EventHandler;







//----------------------------------------------------------------------------------
// hui itself


// images
//int LoadImage(const string &filename);
string set_image(const Image *image, const string &name = "");
void delete_image(const string &name);

// HUI configuration
extern string separator;

};


#include "Callback.h"
#include "Application.h"
#include "main.h"
#include "Event.h"
#include "Menu.h"
#include "Panel.h"
#include "Window.h"
#include "common_dlg.h"
#include "language.h"
#include "Resource.h"
#include "utility.h"
#include "Painter.h"
#include "Toolbar.h"
#include "error.h"
#include "Clipboard.h"


#include "../os/config.h"
namespace hui {
extern Configuration config;
}


#endif


