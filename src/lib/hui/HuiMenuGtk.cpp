#include "hui.h"
#include "hui_internal.h"
#include "Controls/HuiControl.h"
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


GtkAccelGroup *accel_group = NULL;

void try_add_accel(GtkWidget *item, const string &id)
{
	foreach(HuiCommand &c, _HuiCommand_)
		if ((id == c.id) && (c.key_code >= 0)){
			int k = c.key_code;
			int mod = (((k&KEY_SHIFT)>0) ? GDK_SHIFT_MASK : 0) | (((k&KEY_CONTROL)>0) ? GDK_CONTROL_MASK : 0);
			gtk_widget_add_accelerator(item, "activate", accel_group, HuiKeyID[k & 255], (GdkModifierType)mod, GTK_ACCEL_VISIBLE);
		}
}

HuiMenu::HuiMenu()
{
	msg_db_r("HuiMenu()", 1);
	_HuiMakeUsable_();
	win = NULL;
	
	widget = gtk_menu_new();
	if (accel_group == NULL)
		accel_group = gtk_accel_group_new();
	msg_db_l(1);
}

HuiMenu::~HuiMenu()
{
	Clear();
}

void HuiMenu::gtk_realize()
{
	widget = gtk_menu_new();
}

// window coordinate system!
void HuiMenu::OpenPopup(HuiWindow *win, int x, int y)
{
	msg_db_r("CHuiMenu::OpenPopup", 1);
	gtk_widget_show(widget);
	gtk_menu_popup(GTK_MENU(widget), NULL, NULL, NULL, NULL, 0, gtk_get_current_event_time());
	set_win(win);
//	win->popup = this;
	msg_db_l(1);
}

void HuiMenu::add(HuiControl *c)
{
	item.add(c);
	gtk_menu_shell_append(GTK_MENU_SHELL(widget), c->widget);
	gtk_widget_show(c->widget);
	c->panel = win;
	if (win)
		win->control.add(c);
}


const char *get_stock_id(const string image)
{
	if (image=="hui:open")	return GTK_STOCK_OPEN;
	if (image=="hui:new")		return GTK_STOCK_NEW;
	if (image=="hui:save")	return GTK_STOCK_SAVE;
	if (image=="hui:save-as")	return GTK_STOCK_SAVE_AS;
	if (image=="hui:quit")	return GTK_STOCK_QUIT;

	if (image=="hui:copy")	return GTK_STOCK_COPY;
	if (image=="hui:paste")	return GTK_STOCK_PASTE;
	if (image=="hui:cut")		return GTK_STOCK_CUT;
	if (image=="hui:delete")	return GTK_STOCK_DELETE;
	if (image=="hui:close")	return GTK_STOCK_CLOSE;
	if (image=="hui:edit")	return GTK_STOCK_EDIT;
	if (image=="hui:find")	return GTK_STOCK_FIND;
	if (image=="hui:find-and-replace")	return GTK_STOCK_FIND_AND_REPLACE;


	if (image=="hui:no")		return GTK_STOCK_NO;
	if (image=="hui:yes")		return GTK_STOCK_YES;
	if (image=="hui:ok")		return GTK_STOCK_OK;
	if (image=="hui:cancel")	return GTK_STOCK_CANCEL;
	if (image=="hui:apply")	return GTK_STOCK_APPLY;

	if (image=="hui:redo")	return GTK_STOCK_REDO;
	if (image=="hui:undo")	return GTK_STOCK_UNDO;
	if (image=="hui:refresh")	return GTK_STOCK_REFRESH;
	if (image=="hui:preferences")	return GTK_STOCK_PREFERENCES;
	if (image=="hui:properties")return GTK_STOCK_PROPERTIES;

	if (image=="hui:clear")	return GTK_STOCK_CLEAR;
	if (image=="hui:add")		return GTK_STOCK_ADD;
	if (image=="hui:remove")	return GTK_STOCK_REMOVE;
	if (image=="hui:execute")	return GTK_STOCK_EXECUTE;
	if (image=="hui:stop")	return GTK_STOCK_STOP;

	if (image=="hui:up")		return GTK_STOCK_GO_UP;
	if (image=="hui:down")	return GTK_STOCK_GO_DOWN;
	if (image=="hui:back")	return GTK_STOCK_GO_BACK;
	if (image=="hui:forward")	return GTK_STOCK_GO_FORWARD;
	if (image=="hui:bottom")		return GTK_STOCK_GOTO_BOTTOM;
	if (image=="hui:first")	return GTK_STOCK_GOTO_FIRST;
	if (image=="hui:last")	return GTK_STOCK_GOTO_LAST;
	if (image=="hui:top")	return GTK_STOCK_GOTO_TOP;

	if (image=="hui:help")	return GTK_STOCK_HELP;
	if (image=="hui:info")	return GTK_STOCK_INFO;
	if (image=="hui:about")	return GTK_STOCK_ABOUT;
	if (image=="hui:print")	return GTK_STOCK_PRINT;
	if (image=="hui:font")	return GTK_STOCK_SELECT_FONT;
	if (image=="hui:select-all")	return "gtk-select-all";//GTK_STOCK_SELECT_ALL;

	if (image=="hui:zoom-in")	return GTK_STOCK_ZOOM_IN;
	if (image=="hui:zoom-out")	return GTK_STOCK_ZOOM_OUT;
	if (image=="hui:fullscreen")	return GTK_STOCK_FULLSCREEN;
	if (image=="hui:zoom-one")	return GTK_STOCK_ZOOM_100;
	if (image=="hui:zoom-fit")	return GTK_STOCK_ZOOM_FIT;

	
	if (image=="hui:media-play")return GTK_STOCK_MEDIA_PLAY;
	if (image=="hui:media-stop")return GTK_STOCK_MEDIA_STOP;
	if (image=="hui:media-pause")return GTK_STOCK_MEDIA_PAUSE;
	if (image=="hui:media-record")return GTK_STOCK_MEDIA_RECORD;
	if (image=="hui:media-forward")return GTK_STOCK_MEDIA_FORWARD;
	if (image=="hui:media-rewind")return GTK_STOCK_MEDIA_REWIND;
	if (image=="hui:media-next")return GTK_STOCK_MEDIA_NEXT;
	if (image=="hui:media-previous")return GTK_STOCK_MEDIA_PREVIOUS;

	if (image=="hui:connect")	return GTK_STOCK_CONNECT;
	if (image=="hui:disconnect")	return GTK_STOCK_DISCONNECT;
	if (image=="hui:network")	return GTK_STOCK_NETWORK;

	if (image=="hui:error")	return GTK_STOCK_DIALOG_ERROR;
	//if (image=="hui:info")	return GTK_STOCK_DIALOG_INFO;
	if (image=="hui:question")	return GTK_STOCK_DIALOG_QUESTION;
	if (image=="hui:warning")	return GTK_STOCK_DIALOG_WARNING;
	if (image=="hui:authentication")	return GTK_STOCK_DIALOG_AUTHENTICATION;

	if (image=="hui:home")	return GTK_STOCK_HOME;
	if (image=="hui:select-color")	return GTK_STOCK_SELECT_COLOR;
	if (image=="hui:select-font")	return GTK_STOCK_SELECT_FONT;
	if (image=="hui:sort-ascending")	return GTK_STOCK_SORT_ASCENDING;
	if (image=="hui:sort-descending")	return GTK_STOCK_SORT_DESCENDING;
	if (image=="hui:spell-check")	return GTK_STOCK_SPELL_CHECK;
	if (image=="hui:convert")	return GTK_STOCK_CONVERT;
	
	return "";
}

sHuiImage *get_image(const string &image)
{
	foreach(sHuiImage &m, HuiImage)
		if (m.filename == image)
			return &m;
	sHuiImage img = {0, image};
	HuiImage.add(img);
	return &HuiImage.back();
}

void *get_gtk_image(const string &image, bool large)
{
	if (image == "")
		return NULL;
	if (image.find("hui:") == 0){
		// internal
		return gtk_image_new_from_stock(get_stock_id(image), large ? GTK_ICON_SIZE_LARGE_TOOLBAR : GTK_ICON_SIZE_MENU);
	}else{
		// file
		sHuiImage *img = get_image(image);
		if (!img)
			return NULL;
		// absolute path?
		if ((img->filename[0] == '/') || (img->filename[1] == ':'))
			return gtk_image_new_from_file(sys_str_f(img->filename));
		// relative
		return gtk_image_new_from_file(sys_str_f(HuiAppDirectory + img->filename));
	}
}

void *get_gtk_image_pixbuf(const string &image)
{
	if (image.find("hui:") == 0){
		// internal
		GdkPixbuf *pb = gtk_icon_theme_load_icon(gtk_icon_theme_get_default(), get_stock_id(image), 24, (GtkIconLookupFlags)0, NULL);
		if (pb){
			GdkPixbuf *r = gdk_pixbuf_copy(pb);
			g_object_unref(pb);
			return r;
		}
	}else{
		// file
		sHuiImage *img = get_image(image);
		if (img->type == 0){
		}else if (img->type == 1){
#ifdef _X_USE_IMAGE_
			return gdk_pixbuf_new_from_data((guchar*)img->image.data.data, GDK_COLORSPACE_RGB, true, 8, img->image.width, img->image.height, img->image.width * 4, NULL, NULL);
#endif
		}
	}
	return NULL;
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



#endif
