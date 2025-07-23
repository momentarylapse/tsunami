#include "Controls/Control.h"
#include "Controls/MenuItem.h"
#include "Controls/MenuItemSubmenu.h"
#include "Controls/MenuItemToggle.h"
#include "hui.h"
#include "internal.h"
#include "../image/image.h"
#include "../os/msg.h"
#ifdef HUI_API_GTK

#include <gtk/gtk.h>

#include "../os/app.h"

#ifdef HUGE
#undef HUGE
#endif

namespace hui
{


#if !GTK_CHECK_VERSION(4,0,0)
GtkAccelGroup *accel_group = nullptr;

int key_to_gtk(int key_code, GdkModifierType* mod);

void try_add_accel(GtkWidget *item, const string &id, Panel *panel) {
	if (!panel->win)
		return;
	for (auto &c: panel->win->get_event_key_codes())
		if ((id == c.id) and (c.key_code >= 0)) {
			GdkModifierType mod;
			int gtk_key = key_to_gtk(c.key_code, &mod);
			gtk_widget_add_accelerator(item, "activate", accel_group, gtk_key, mod, GTK_ACCEL_VISIBLE);
		}
}
#endif

Menu::Menu(Panel *p) {
	_MakeUsable_();
	panel = p;

#if GTK_CHECK_VERSION(4,0,0)
	gmenu = g_menu_new();
#else
	widget = gtk_menu_new();
	if (accel_group == nullptr)
		accel_group = gtk_accel_group_new();
#endif
}

Menu::~Menu() {
	clear();
}

void Menu::gtk_realize() {
#if !GTK_CHECK_VERSION(4,0,0)
	widget = gtk_menu_new();
#endif
}


#if !GTK_CHECK_VERSION(4,0,0)
void OnGtkMenuClose(GtkMenuShell *menushell, gpointer user_data) {
	Menu *m = (Menu*)user_data;
	m->set_panel(nullptr);
}
#endif


// window coordinate system!
void Menu::open_popup(Panel *panel) {
#if GTK_CHECK_VERSION(4,0,0)
	// transform into panel coordinates
	int dx = 0, dy = 0;
	panel->apply_foreach(panel->_get_cur_id_(), [panel, &dx, &dy] (Control *c) {
		graphene_point_t A{0,0}, B;
		if (gtk_widget_compute_point (c->widget, panel->root_control->widget, &A, &B)) {
			dx = (int)B.x;
			dy = (int)B.y;
		}
	});


	panel->_connect_menu_to_panel(this);

	auto w = gtk_popover_menu_new_from_model(G_MENU_MODEL(gmenu));

	gtk_widget_set_parent(w, panel->root_control->widget);
	GdkRectangle rr = {(int)panel->win->input.x + dx, (int)panel->win->input.y + dy, 0, 0};
	// TODO add widget offset relative to panel
	gtk_popover_set_pointing_to(GTK_POPOVER(w), &rr);
	gtk_popover_popup(GTK_POPOVER(w));


#else
	gtk_widget_show(widget);
#if GTK_CHECK_VERSION(3,22,0)
	gtk_menu_popup_at_pointer(GTK_MENU(widget), nullptr);
#else
	gtk_menu_popup(GTK_MENU(widget), nullptr, nullptr, nullptr, nullptr, 0, gtk_get_current_event_time());
#endif

	//g_signal_connect(G_OBJECT(widget), "selection-done", G_CALLBACK(&OnGtkMenuClose), this);
	set_panel(panel);
#endif
}

void Menu::_add(shared<Control> c) {
#if GTK_CHECK_VERSION(4,0,0)
	if ((c->type == MENU_ITEM_BUTTON) or (c->type == MENU_ITEM_TOGGLE) or (c->type == MENU_ITEM_SUBMENU))
		g_menu_append_item(gmenu, dynamic_cast<BasicMenuItem*>(c.get())->item);
	items.add(c);
	//c->panel = panel;
#else
	items.add(c);
	gtk_menu_shell_append(GTK_MENU_SHELL(widget), c->widget);
	gtk_widget_show(c->widget);
	c->panel = panel;
#endif
}


const char *get_gtk_icon_name_base(const string image) {
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

	if (image.head(4) == "hui:")
		return image.sub_ref(4).c_str();
	return "";
}

string get_gtk_icon_name(const string _image) {
	string image = _image.replace("-symbolic", "");
	string post = "-symbolic"; //(_image.find("-symbolic", 0) >= 0) ? "-symbolic" : "";
	return get_gtk_icon_name_base(image) + post;
}

HuiImage *get_image(const string &filename) {
	for (HuiImage &m: _all_images_)
		if (m.filename == filename)
			return &m;
	if (filename != "")
		msg_error("hui.get_image(): missing image: " + filename);
	_all_images_.add({0, filename, new Image(16, 16, Red), nullptr});
	return &_all_images_.back();
}

GtkIconTheme *get_hui_icon_theme() {
	static GtkIconTheme *hui_icon_theme = nullptr;
	if (!hui_icon_theme) {
		hui_icon_theme = gtk_icon_theme_new();
#if GTK_CHECK_VERSION(4,0,0)
		gtk_icon_theme_add_search_path(hui_icon_theme, sys_str_f(os::app::directory_static | "icons"));
		/*gtk_icon_theme_add_search_path(hui_icon_theme, sys_str_f(Application::directory_static | "icons" | "64x64"));
		gtk_icon_theme_add_search_path(hui_icon_theme, sys_str_f(Application::directory_static | "icons" | "48x48"));
		gtk_icon_theme_add_search_path(hui_icon_theme, sys_str_f(Application::directory_static | "icons" | "32x32"));*/
#else
		gtk_icon_theme_append_search_path(hui_icon_theme, sys_str_f(Application::directory_static | "icons"));
		gtk_icon_theme_append_search_path(hui_icon_theme, sys_str_f(Application::directory_static | "icons" | "64x64"));
		gtk_icon_theme_append_search_path(hui_icon_theme, sys_str_f(Application::directory_static | "icons" | "48x48"));
		gtk_icon_theme_append_search_path(hui_icon_theme, sys_str_f(Application::directory_static | "icons" | "32x32"));
#endif
		// guess, only 64x64 is used... could skip the others
	}
	return hui_icon_theme;
}

int absolute_icon_size(IconSize s) {
	if (s == IconSize::REGULAR)
		return 16;
	if (s == IconSize::LARGE)
		return 32;
	if (s == IconSize::HUGE)
		return 48;
	if (s == IconSize::TOOLBAR_LARGE)
		return 24;//32;
	if (s == IconSize::TOOLBAR_SMALL)
		return 16;
	return s;
}

int icon_size_gtk(IconSize s) {
	if (s == IconSize::REGULAR) {
#if GTK_CHECK_VERSION(4,0,0)
		return GTK_ICON_SIZE_NORMAL;
#else
		return GTK_ICON_SIZE_BUTTON;
#endif
	} else if (s == IconSize::LARGE) {
#if GTK_CHECK_VERSION(4,0,0)
		return GTK_ICON_SIZE_LARGE;
#else
		return GTK_ICON_SIZE_DND;
#endif
	} else if (s == IconSize::HUGE) {
#if GTK_CHECK_VERSION(4,0,0)
		return GTK_ICON_SIZE_LARGE;
#else
		return GTK_ICON_SIZE_DIALOG;
#endif
	} else if (s == IconSize::TOOLBAR_LARGE) {
#if GTK_CHECK_VERSION(4,0,0)
		return GTK_ICON_SIZE_LARGE;
#else
		return GTK_ICON_SIZE_LARGE_TOOLBAR;
#endif
	} else if (s == IconSize::TOOLBAR_SMALL) {
#if GTK_CHECK_VERSION(4,0,0)
		return GTK_ICON_SIZE_LARGE;
#else
		return GTK_ICON_SIZE_SMALL_TOOLBAR;
#endif
	}
	return 0;
}

GtkWidget *get_gtk_image_x(const string &image, IconSize size, GtkWidget *widget) {
	if (image == "")
		return nullptr;
	if (image.head(4) == "hui:") {
		// internal
#if GTK_CHECK_VERSION(4,0,0)
		auto iii = gtk_image_new_from_icon_name(get_gtk_icon_name(image).c_str());
		gtk_image_set_pixel_size(GTK_IMAGE(iii), absolute_icon_size(size));
		return iii;
#else
		return gtk_image_new_from_icon_name(get_gtk_icon_name(image).c_str(), (GtkIconSize)icon_size_gtk(size));
#endif
	} else if (image.find(".") < 0) {
		int _size = absolute_icon_size(size);
		auto theme = get_hui_icon_theme();
#if GTK_CHECK_VERSION(4,0,0)
		//msg_write("SYMBOLIC IMAGE " + image);
		//return gtk_image_new_from_icon_name(get_gtk_icon_name("hui:open").c_str());

		auto info = gtk_icon_theme_lookup_icon(theme, image.c_str(), nullptr, _size, 1, GTK_TEXT_DIR_NONE, GTK_ICON_LOOKUP_FORCE_SYMBOLIC);
		if (!info) {
			msg_write("nope");
			info = gtk_icon_theme_lookup_icon(theme, image.c_str(), nullptr, _size, 1, GTK_TEXT_DIR_NONE, (GtkIconLookupFlags)0);//, GTK_ICON_LOOKUP_FORCE_SYMBOLIC);
		}

		// TODO use theme???
		auto iii =gtk_image_new_from_paintable(GDK_PAINTABLE(info));
		gtk_image_set_pixel_size(GTK_IMAGE(iii), _size);
		return iii;
#else
		auto info = gtk_icon_theme_lookup_icon(theme, image.c_str(), _size, GTK_ICON_LOOKUP_FORCE_SYMBOLIC);
		if (!info)
			info = gtk_icon_theme_lookup_icon(theme, image.c_str(), _size, (GtkIconLookupFlags)0);//, GTK_ICON_LOOKUP_FORCE_SYMBOLIC);
		auto sc = gtk_widget_get_style_context(widget);

		gboolean was_sym = true;
		GError *error = nullptr;
		auto pixbuf = gtk_icon_info_load_symbolic_for_context(info, sc, &was_sym, &error);
		if (error) {
			msg_error(error->message);
		}
		return gtk_image_new_from_pixbuf(pixbuf);
		//return gtk_image_new_from_file(sys_str_f(Application::directory_static << image.sub_ref(7)));
#endif
	} else {
		// file
		//HuiImage *img = get_image(image);
		//if (!img)
		//	return NULL;
		// absolute path?
	//	if ((img->filename[0] == '/') or (img->filename[1] == ':'))
	//		return gtk_image_new_from_file(sys_str_f(img->filename));
		// relative
		return gtk_image_new_from_file(sys_str_f(os::app::directory_static | image));
	}
}
void *get_gtk_image(const string &image, IconSize size) {
	return get_gtk_image_x(image, size, nullptr);
}

#if GTK_CHECK_VERSION(4,0,0)
void *get_gtk_image_paintable(const string &image) {
	if (image.head(4) == "hui:") {
		// internal
		auto icon_theme = gtk_icon_theme_get_for_display(gdk_display_get_default());
		if (auto paintable = gtk_icon_theme_lookup_icon(icon_theme, get_gtk_icon_name(image).c_str(), nullptr, 24, 1, GTK_TEXT_DIR_LTR, (GtkIconLookupFlags)0))
			return paintable;
	} else {
		// file
		HuiImage *img = get_image(image);
		if (img->type == 0) {
		} else if (img->type == 1) {
			if (!img->paintable) {
				img->pix_buf = gdk_pixbuf_new_from_data((guchar*)img->image->data.data, GDK_COLORSPACE_RGB, true, 8, img->image->width, img->image->height, img->image->width * 4, nullptr, nullptr);
				img->paintable = gdk_texture_new_for_pixbuf(GDK_PIXBUF(img->pix_buf));
				for (int i=0; i<1000; i++)
					g_object_ref(GDK_TEXTURE(img->paintable));
			}
			return img->paintable;
		}
	}
	return nullptr;
}
#else
void *get_gtk_image_paintable(const string &image) {
	if (image.head(4) == "hui:") {
		// internal
		GdkPixbuf *pb = gtk_icon_theme_load_icon(gtk_icon_theme_get_default(), get_gtk_icon_name(image).c_str(), 24, (GtkIconLookupFlags)0, nullptr);
		if (pb) {
			GdkPixbuf *r = gdk_pixbuf_copy(pb);
			g_object_unref(pb);
			return r;
		}
	} else {
		// file
		HuiImage *img = get_image(image);
		if (img->type == 0) {
		} else if (img->type == 1) {
			if (!img->pix_buf) {
				img->pix_buf = gdk_pixbuf_new_from_data((guchar*)img->image->data.data, GDK_COLORSPACE_RGB, true, 8, img->image->width, img->image->height, img->image->width * 4, nullptr, nullptr);
				for (int i=0; i<1000; i++)
					g_object_ref(GDK_PIXBUF(img->pix_buf));
			}
			return img->pix_buf;
		}
	}
	return nullptr;
}
#endif

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
