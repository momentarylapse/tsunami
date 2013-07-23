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

#include "hui_common.h"
#ifdef _X_USE_IMAGE_
	#include "../image/image.h"
#else
	typedef int Image;
	typedef int color;
#endif

extern string HuiVersion;




class HuiWindow;
class HuiMenu;
class HuiEventHandler;







//----------------------------------------------------------------------------------
// hui itself


extern bool HuiEndKeepMsgAlive;

// images
//int HuiLoadImage(const string &filename);
string HuiSetImage(const Image &image);

// HUI configuration
extern string HuiComboBoxSeparator;

// data from hui (...don't change...)
extern string HuiAppFilename, HuiAppDirectory, HuiAppDirectoryStatic;
extern string HuiInitialWorkingDirectory;
extern bool HuiRunning;



#include "hui_main.h"
#include "HuiMenu.h"
#include "HuiWindow.h"
#include "hui_common_dlg.h"
#include "hui_language.h"
#include "hui_config.h"
#include "hui_input.h"
#include "hui_resource.h"
#include "hui_utility.h"
#include "HuiPainter.h"
#include "HuiTimer.h"
#include "HuiToolbar.h"
#include "hui_error.h"
#include "hui_clipboard.h"


#endif


