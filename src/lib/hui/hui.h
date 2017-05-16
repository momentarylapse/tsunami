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
#ifdef _X_USE_IMAGE_
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
string SetImage(const Image &image);
void DeleteImage(const string &name);

// HUI configuration
extern string ComboBoxSeparator;

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
#include "Config.h"
#include "Resource.h"
#include "utility.h"
#include "Painter.h"
#include "Timer.h"
#include "Toolbar.h"
#include "error.h"
#include "Clipboard.h"


#endif


