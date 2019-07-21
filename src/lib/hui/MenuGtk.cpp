#include "Controls/Control.h"
#include "hui.h"
#include "internal.h"
#ifdef HUI_API_GTK


/*#include "../file/file.h"
#include <stdio.h>
#include <signal.h>
#ifdef HUI_API_WIN
	#include <shlobj.h>
	#include <winuser.h>
	#include <direct.h>
	#include <commctrl.h>
	#include <tchar.h>
	#pragma comment(lib,"winmm.lib")
	#pragma warning(disable : 4995)
#endif
#ifdef OS_LINUX
	#include <string.h>
	#include <unistd.h>
	#include <sys/time.h>
	#include <sys/timeb.h>
	#include <time.h>
	#include <gdk/gdkx.h>
#endif*/

namespace hui
{


GtkAccelGroup *accel_group = nullptr;

void try_add_accel(GtkWidget *item, const string &id, Panel *panel)
{
	for (auto &c: panel->event_key_codes)
		if ((id == c.id) and (c.key_code >= 0)){
			int k = c.key_code;
			int mod = (((k&KEY_SHIFT)>0) ? GDK_SHIFT_MASK : 0) | (((k&KEY_CONTROL)>0) ? GDK_CONTROL_MASK : 0) | (((k&KEY_ALT)>0) ? GDK_META_MASK : 0);
			gtk_widget_add_accelerator(item, "activate", accel_group, HuiKeyID[k & 255], (GdkModifierType)mod, GTK_ACCEL_VISIBLE);
		}
}

Menu::Menu()
{
	_MakeUsable_();
	panel = nullptr;
	
	widget = gtk_menu_new();
	if (accel_group == nullptr)
		accel_group = gtk_accel_group_new();
}

Menu::~Menu()
{
	clear();
}

void Menu::gtk_realize()
{
	widget = gtk_menu_new();
}


void OnGtkMenuClose(GtkMenuShell *menushell, gpointer user_data)
{
	Menu *m = (Menu*)user_data;
	m->set_panel(nullptr);
}


// window coordinate system!
void Menu::open_popup(Panel *panel)
{
	gtk_widget_show(widget);
#if GTK_CHECK_VERSION(3,22,0)
	gtk_menu_popup_at_pointer(GTK_MENU(widget), nullptr);
#else
	gtk_menu_popup(GTK_MENU(widget), nullptr, nullptr, nullptr, nullptr, 0, gtk_get_current_event_time());
#endif

	//g_signal_connect(G_OBJECT(widget), "selection-done", G_CALLBACK(&OnGtkMenuClose), this);
	set_panel(panel);
}

void Menu::_add(Control *c)
{
	items.add(c);
	gtk_menu_shell_append(GTK_MENU_SHELL(widget), c->widget);
	gtk_widget_show(c->widget);
	c->panel = panel;
}


const char *get_gtk_icon_name(const string image)
{
	if (image=="hui:open")	return "document-open";
	if (image=="hui:new")		return "document-new";
	if (image=="hui:save")	return "document-save";
	if (image=="hui:save-as")	return "document-save-as";
	if (image=="hui:quit")	return "application-exit";

	if (image=="hui:copy")	return "edit-copy";
	if (image=="hui:paste")	return "edit-paste";
	if (image=="hui:cut")		return "edit-cut";
	if (image=="hui:delete")	return "edit-delete";
	if (image=="hui:close")	return "window-close";
	if (image=="hui:edit")	return "document-edit"; // ???
	if (image=="hui:find")	return "edit-find";
	if (image=="hui:find-and-replace")	return "edit-find-replace";


	if (image=="hui:no")		return "dialog-no";//GTK_STOCK_NO;
	if (image=="hui:yes")		return "dialog-yes";//GTK_STOCK_YES;
	if (image=="hui:ok")		return "dialog-ok";//GTK_STOCK_OK;
	if (image=="hui:cancel")	return "dialog-cancel";//GTK_STOCK_CANCEL;
	if (image=="hui:apply")	return "dialog-apply";//GTK_STOCK_APPLY;

	if (image=="hui:redo")	return "edit-redo";
	if (image=="hui:undo")	return "edit-undo";
	if (image=="hui:refresh")	return "view-refresh";
	if (image=="hui:preferences")	return "preferences-system";
	if (image=="hui:properties")return "document-properties";

	if (image=="hui:clear")	return "edit-clear";
	if (image=="hui:add")		return "list-add";
	if (image=="hui:remove")	return "list-remove";
	if (image=="hui:execute")	return "system-run";
	if (image=="hui:stop")	return "process-stop";

	if (image=="hui:up")		return "go-up";
	if (image=="hui:down")	return "go-down";
	if (image=="hui:back")	return "go-previous";
	if (image=="hui:forward")	return "go-next";
	if (image=="hui:bottom")		return "go-bottom";
	if (image=="hui:first")	return "go-first";
	if (image=="hui:last")	return "go-last";
	if (image=="hui:top")	return "go-top";

	if (image=="hui:help")	return "help-browser";
	if (image=="hui:info")	return "dialog-information";
	if (image=="hui:about")	return "help-about";
	if (image=="hui:print")	return "document-print";
	if (image=="hui:font")	return "font";//GTK_STOCK_SELECT_FONT;
	if (image=="hui:select-all")	return "edit-select-all";

	if (image=="hui:zoom-in")	return "zoom-in";
	if (image=="hui:zoom-out")	return "zoom-out";
	if (image=="hui:fullscreen")	return "view-fullscreen";
	if (image=="hui:zoom-one")	return "zoom-original";
	if (image=="hui:zoom-fit")	return "zoom-fit-best";


	if (image=="hui:media-play")return "media-playback-start";
	if (image=="hui:media-stop")return "media-playback-stop";
	if (image=="hui:media-pause")return "media-playback-pause";
	if (image=="hui:media-record")return "media-record";
	if (image=="hui:media-forward")return "media-seek-forward";
	if (image=="hui:media-rewind")return "media-seek-backward";
	if (image=="hui:media-next")return "media-skip-forward";
	if (image=="hui:media-previous")return "media-skip-backward";

	if (image=="hui:connect")	return "network-connect";//GTK_STOCK_CONNECT;
	if (image=="hui:disconnect")	return "network-disconnect";//GTK_STOCK_DISCONNECT;
	if (image=="hui:network")	return "network";//GTK_STOCK_NETWORK;

	if (image=="hui:error")	return "dialog-error";
	//if (image=="hui:info")	return "dialog-information";
	if (image=="hui:question")	return "dialog-question";
	if (image=="hui:warning")	return "dialog-warning";
	if (image=="hui:authentication")	return "dialog-password";

	if (image=="hui:home")	return "go-home";
	if (image=="hui:select-color")	return "color-picker";//GTK_STOCK_SELECT_COLOR;
	if (image=="hui:select-font")	return "font";//GTK_STOCK_SELECT_FONT;
	if (image=="hui:sort-ascending")	return "view-sort-ascending";
	if (image=="hui:sort-descending")	return "view-sort-descending";
	if (image=="hui:spell-check")	return "tools-check-spelling";
	if (image=="hui:convert")	return "gtk-convert";//GTK_STOCK_CONVERT;

	return "";
}

HuiImage *get_image(const string &filename)
{
	for (HuiImage &m: _all_images_)
		if (m.filename == filename)
			return &m;
	HuiImage img = {0, filename};
	_all_images_.add(img);
	return &_all_images_.back();
}

void *get_gtk_image(const string &image, bool large)
{
	if (image == "")
		return nullptr;
	if (image.head(4) == "hui:"){
		// internal
		return gtk_image_new_from_icon_name(get_gtk_icon_name(image), large ? GTK_ICON_SIZE_LARGE_TOOLBAR : GTK_ICON_SIZE_MENU);
	}else{
		// file
		//HuiImage *img = get_image(image);
		//if (!img)
		//	return NULL;
		// absolute path?
	//	if ((img->filename[0] == '/') or (img->filename[1] == ':'))
	//		return gtk_image_new_from_file(sys_str_f(img->filename));
		// relative
		return gtk_image_new_from_file(sys_str_f(Application::directory_static + image));
	}
}

void *get_gtk_image_pixbuf(const string &image)
{
	if (image.head(4) == "hui:"){
		// internal
		GdkPixbuf *pb = gtk_icon_theme_load_icon(gtk_icon_theme_get_default(), get_gtk_icon_name(image), 24, (GtkIconLookupFlags)0, nullptr);
		if (pb){
			GdkPixbuf *r = gdk_pixbuf_copy(pb);
			g_object_unref(pb);
			return r;
		}
	}else{
		// file
		HuiImage *img = get_image(image);
		if (img->type == 0){
		}else if (img->type == 1){
#ifdef _X_USE_IMAGE_
			return gdk_pixbuf_new_from_data((guchar*)img->image.data.data, GDK_COLORSPACE_RGB, true, 8, img->image.width, img->image.height, img->image.width * 4, nullptr, nullptr);
#endif
		}
	}
	return nullptr;
}

/*void HuiMenu::SetText(const string &id, const string &text)
{
	foreach(HuiMenuItem &it, item){
		if (it.sub_menu)
			it.sub_menu->SetText(id, text);
		if (it.id == id){
			it.name = text;
			
#ifndef OS_WINDOWS
			gtk_menu_item_set_label(GTK_MENU_ITEM(it.widget), sys_str(text));
#endif
			try_add_accel(it.widget, id);
		}
	}
}*/


};

#endif
