/*----------------------------------------------------------------------------*\
| CHui                                                                         |
| -> Heroic User Interface                                                     |
| -> abstraction layer for GUI                                                 |
|   -> Windows (default api) or Linux (Gtk+)                                   |
|                                                                              |
| vital properties:                                                            |
|                                                                              |
| last updated: 2010.06.21 (c) by MichiSoft TM                                 |
\*----------------------------------------------------------------------------*/


#include "hui.h"
#include "../image/image.h"
#include "../os/config.h"
#include "../os/file.h"
#include "../os/time.h"
#include "../os/msg.h"

#include <gtk/gtk.h>

#include <stdio.h>
#include <signal.h>

#include "internal.h"
#ifdef OS_WINDOWS
	#include <shlobj.h>
	#include <winuser.h>
	#include <direct.h>
	#include <commctrl.h>
	#include <tchar.h>
	#pragma warning(disable : 4995)
#endif
#if defined(OS_LINUX) || defined(OS_MAC)
#if HAS_LIB_XLIB
#if !GTK_CHECK_VERSION(4,0,0)
	#include <gdk/gdkx.h>
#endif
#endif
#endif

#if HAS_LIB_ADWAITA && GTK_CHECK_VERSION(4,0,0)
#include <adwaita.h>
#endif

namespace hui
{


string Version = "0.7.6.4";


#ifdef OS_WINDOWS
	void *hui_win_instance;
#endif
#ifdef HUI_API_WIN
	HFONT hui_win_default_font;
	HICON hui_win_main_icon;
#endif
#if defined(OS_LINUX) || defined(OS_MAC)
#if !GTK_CHECK_VERSION(4,0,0)
	Display* x_display;
#endif
#endif




bool _screen_opened_ = false;

// HUI configuration
string separator;




#ifdef HUI_API_GTK
	void *invisible_cursor = nullptr;
#endif

Array<HuiImage> _all_images_;


Configuration config;


}


namespace hui
{

void _init_global_css_classes_();
void init_low_level();

void _MakeUsable_() {
	if (_screen_opened_)
		return;

	init_low_level();

	if (true) { //(Application::flags & Flags::LAZY_GUI_INITIALIZATION) == 0) {
#if GTK_CHECK_VERSION(4,0,0)
#if HAS_LIB_ADWAITA
		if (config.get_bool("hui.allow-libadwaita", true)) {
			os::Timer timer;
			adw_init();
			if (timer.get() > 4)
				msg_error("libadwaita adw_init() takes too long... this happens on some desktop environments.\nTry setting 'hui.allow-libadwaita = false' in 'config.txt'");
			Application::adwaita_started = true;
		}
#endif
		gtk_init();
		if (Application::allowed)
			Application::application = gtk_application_new(nullptr, G_APPLICATION_NON_UNIQUE);
#else
		gtk_init(nullptr, nullptr);
		_init_global_css_classes_();
#endif
	} else {
		// direct init
#if GTK_CHECK_VERSION(4,0,0)
		gtk_init();
#else
		gtk_init(nullptr, nullptr);
#endif
		_init_global_css_classes_();
	}


#if defined(OS_LINUX) || defined(OS_MAC)
#if !GTK_CHECK_VERSION(4,0,0)
#if HAS_LIB_XLIB
		x_display = XOpenDisplay(0);
#endif
#endif
#endif

#if !GTK_CHECK_VERSION(4,0,0)
#if GTK_CHECK_VERSION(3,16,0)
	invisible_cursor = gdk_cursor_new_for_display(gdk_display_get_default(), GDK_BLANK_CURSOR);
#else
	invisible_cursor = gdk_cursor_new(GDK_BLANK_CURSOR);
#endif
#endif

	_screen_opened_ = true;
}




static int _current_image_no_ = 0;

string set_image(const Image *image, const string &user_name) {
	HuiImage *img = nullptr;
	if (user_name != "") {
		if (user_name.head(6) != "image:") {
			msg_error("hui.set_image(): name must begin with 'image:'");
			return "";
		}
		for (int i=0;i<_all_images_.num;i++)
			if (_all_images_[i].filename == user_name) {
				img = &_all_images_[i];
				g_object_unref(GDK_PIXBUF(img->pix_buf));
				img->pix_buf = nullptr;
			}
	}
	if (!img) {
		HuiImage _img;
		_img.type = 1;
		_img.filename = format("image:%d", _current_image_no_ ++);
		_img.pix_buf = nullptr;
		if (user_name != "")
			_img.filename = user_name;
		_all_images_.add(_img);
		img = &_all_images_.back();
	}
	img->image = new Image();
	*img->image = *image;
	return img->filename;
}

void delete_image(const string &name) {
	for (int i=0;i<_all_images_.num;i++)
		if (_all_images_[i].filename == name) {
			if (_all_images_[i].pix_buf)
				g_object_unref(GDK_PIXBUF(_all_images_[i].pix_buf));
			delete _all_images_[i].image;
			_all_images_.erase(i);
		}
}


};
