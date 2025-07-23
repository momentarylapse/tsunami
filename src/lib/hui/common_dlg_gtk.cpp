#include "hui.h"

#include "../base/optional.h"
#include "../image/color.h"
#include "../os/app.h"
#include "../os/file.h"
#include "../os/filesystem.h"
#include "../os/msg.h"

#include <gtk/gtk.h>

namespace hui
{
extern bool color_button_linear;

struct DialogParams {
	string filter, show_filter;
	base::optional<string> font;
	base::optional<string> default_name;
	base::optional<string> default_extension;
	bool multiple_files = false;

	static DialogParams parse(const Array<string> &params) {
		DialogParams p;
		for (auto &a: params) {
			int pp = a.find("=");
			if (pp < 0)
				pp = a.num;
			string key = a.head(pp).replace("-", "");
			string value = a.sub_ref(pp + 1);
			if (key == "filter")
				p.filter = value;
			else if ((key == "filtershow") or (key == "showfilter"))
				p.show_filter = value;
			else if (key == "multiple")
				p.multiple_files = true;
			else if (key == "font")
				p.font = value;
			else if (key == "default")
				p.default_name = value;
			else if (key == "defaultextension")
				p.default_extension = value;
			else
				msg_error(format("unknown key '%s'", key));
		}
		return p;
	}

	static DialogParams STATIC_PARAMS;

	Path try_to_ensure_extension(const Path &filename) const {
		if (filename.extension() == "" and default_extension)
			return filename.with("." + *default_extension);

		return filename;

		/*if (filter.find(".") < 0)
			return filename;

		// multiple choices -> ignore
		if (filter.find(";") >= 0)
			return filename;

		string filter_ext = filter.lower().replace("*.", ""); // "*.ext"

		// not the wanted extension -> add
		if (filename.extension() != filter_ext)
			return Path(filename.str() + "." + filter_ext);
		return filename;*/
	}
};
DialogParams DialogParams::STATIC_PARAMS;


static GtkWindow *get_window_save(Window *win) {
	_MakeUsable_();
	return win ? GTK_WINDOW(win->window) : nullptr;
}


#if !GTK_CHECK_VERSION(4,0,0)
	void gtk_window_destroy(GtkWindow *win) {
		gtk_widget_destroy(GTK_WIDGET(win));
	}
#endif

static base::promise<Path> cur_file_promise;

#if GTK_CHECK_VERSION(4,10,0)
static void on_file_dialog_open(GObject* o, GAsyncResult* res, gpointer user_data) {
	auto dlg = GTK_FILE_DIALOG(o);
	if (auto f = gtk_file_dialog_open_finish(dlg, res, nullptr))
		cur_file_promise(g_file_get_path(f));
	else
		cur_file_promise.fail();
}
static void on_file_dialog_save(GObject* o, GAsyncResult* res, gpointer user_data) {
	auto dlg = GTK_FILE_DIALOG(o);
	if (auto f = gtk_file_dialog_save_finish(dlg, res, nullptr))
		cur_file_promise(g_file_get_path(f));
	else
		cur_file_promise.fail();
}
static void on_file_dialog_dir(GObject* o, GAsyncResult* res, gpointer user_data) {
	auto dlg = GTK_FILE_DIALOG(o);
	if (auto f = gtk_file_dialog_select_folder_finish(dlg, res, nullptr))
		cur_file_promise(g_file_get_path(f));
	else
		cur_file_promise.fail();
}

static void add_filters(GtkFileDialog *dlg, const DialogParams &p) {
	auto ls = g_list_store_new(G_TYPE_OBJECT);
	Array<string> show_filter_list = p.show_filter.explode("|");
	Array<string> filter_list = p.filter.explode("|");
	for (int i=0; i<min(show_filter_list.num, filter_list.num); i++){
		GtkFileFilter *gtk_filter = gtk_file_filter_new();
		gtk_file_filter_set_name(gtk_filter, sys_str(show_filter_list[i]));
		auto filters = filter_list[i].explode(";");
		for (string &f: filters)
			gtk_file_filter_add_pattern(gtk_filter, sys_str(f));
		g_list_store_append(ls, gtk_filter);
	}
	gtk_file_dialog_set_filters(dlg, G_LIST_MODEL(ls));
}
#else
#if GTK_CHECK_VERSION(4,0,0)
static void on_gtk_file_dialog_response(GtkDialog *self, gint response_id, gpointer user_data) {
	if (response_id == GTK_RESPONSE_ACCEPT) {
		auto fn = gtk_file_chooser_get_file(GTK_FILE_CHOOSER(self));
		gtk_window_destroy(GTK_WINDOW(self));
		cur_file_promise(DialogParams::STATIC_PARAMS.try_to_ensure_extension(g_file_get_path(fn)));
	} else {
		gtk_window_destroy(GTK_WINDOW(self));
		cur_file_promise.fail();
	}
}
#endif

static void add_filters(GtkWidget *dlg, const string &show_filter, const string &filter) {
	Array<string> show_filter_list = show_filter.explode("|");
	Array<string> filter_list = filter.explode("|");
	for (int i=0; i<min(show_filter_list.num, filter_list.num); i++){
		GtkFileFilter *gtk_filter = gtk_file_filter_new();
		gtk_file_filter_set_name(gtk_filter, sys_str(show_filter_list[i]));
		auto filters = filter_list[i].explode(";");
		for (string &f: filters)
			gtk_file_filter_add_pattern(gtk_filter, sys_str(f));
		gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dlg), gtk_filter);
	}
}
#endif

// choose a directory (<dir> will be selected initially)
FileFuture file_dialog_dir(Window *win, const string &title, const Path &dir, const Array<string> &params) {
	cur_file_promise.reset();
	auto p = DialogParams::parse(params);
	GtkWindow *w = get_window_save(win);
#if GTK_CHECK_VERSION(4,10,0)
	auto dlg = gtk_file_dialog_new();
	gtk_file_dialog_set_title(dlg, sys_str(title));
	if (dir)
		gtk_file_dialog_set_initial_folder(dlg, g_file_new_for_path(dir.str().c_str()));
	gtk_file_dialog_select_folder(dlg, w, nullptr, &on_file_dialog_dir, nullptr);
#elif GTK_CHECK_VERSION(4,0,0)
	GtkWidget* dlg = gtk_file_chooser_dialog_new(sys_str(title),
												w,
												GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER,
												_("Cancel").c_str(),	GTK_RESPONSE_CANCEL,
												_("Open").c_str(),		GTK_RESPONSE_ACCEPT,
												nullptr);
	if (dir)
		gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dlg), g_file_new_for_path(dir.str().c_str()), nullptr);
	g_signal_connect(dlg, "response", G_CALLBACK(on_gtk_file_dialog_response), nullptr);
	gtk_window_present(GTK_WINDOW(dlg));
#else
	GtkWidget* dlg = gtk_file_chooser_dialog_new(sys_str(title),
												w,
												GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER,
												"gtk-cancel",	GTK_RESPONSE_CANCEL,
												"gtk-open",		GTK_RESPONSE_ACCEPT,
												nullptr);
	if (dir)
		gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dlg), dir.str().c_str());
	int r = gtk_dialog_run(GTK_DIALOG(dlg));
	if (r == GTK_RESPONSE_ACCEPT) {
		auto fn = Path(gtk_file_chooser_get_current_folder(GTK_FILE_CHOOSER(dlg))).as_dir();
		gtk_widget_destroy(dlg);
		run_later(0.01f, [fn] {
			cur_file_promise(fn);
		});
	} else {
		gtk_widget_destroy(dlg);
	}
#endif
	return cur_file_promise.get_future();
}

// file selection for opening (filter should look like "*.txt")
FileFuture file_dialog_open(Window *win, const string &title, const Path &dir, const Array<string> &params) {
	cur_file_promise.reset();
	auto p = DialogParams::parse(params);
	DialogParams::STATIC_PARAMS = p;
	GtkWindow *w = get_window_save(win);
#if GTK_CHECK_VERSION(4,10,0)
	auto dlg = gtk_file_dialog_new();
	gtk_file_dialog_set_title(dlg, sys_str(title));
	if (dir)
		gtk_file_dialog_set_initial_folder(dlg, g_file_new_for_path(dir.str().c_str()));
	add_filters(dlg, p);
	/*if (p.multiple_files)
		gtk_file_dialog_open_multiple(dlg, w, nullptr, &on_file_dialog_open_multi, nullptr);
	else*/
		gtk_file_dialog_open(dlg, w, nullptr, &on_file_dialog_open, nullptr);
#elif GTK_CHECK_VERSION(4,0,0)
	GtkWidget *dlg = gtk_file_chooser_dialog_new(sys_str(title),
												w,
												GTK_FILE_CHOOSER_ACTION_OPEN,
												_("Cancel").c_str(),	GTK_RESPONSE_CANCEL,
												_("Open").c_str(),		GTK_RESPONSE_ACCEPT,
												nullptr);
	gtk_window_set_modal(GTK_WINDOW(dlg), true);
	if (p.multiple_files)
		gtk_file_chooser_set_select_multiple(GTK_FILE_CHOOSER(dlg), true);
	if (dir) {
		auto path = g_file_new_for_path(dir.c_str());
		gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dlg), path, nullptr);
		//g_object_unref(path);
	}
	add_filters(dlg, p.show_filter, p.filter);
	g_signal_connect(dlg, "response", G_CALLBACK(on_gtk_file_dialog_response), nullptr);
	gtk_window_present(GTK_WINDOW(dlg));

#else
	GtkWidget *dlg = gtk_file_chooser_dialog_new(sys_str(title),
												w,
												GTK_FILE_CHOOSER_ACTION_OPEN,
												"gtk-cancel",	GTK_RESPONSE_CANCEL,
												"gtk-open",		GTK_RESPONSE_ACCEPT,
												nullptr);
	gtk_window_set_modal(GTK_WINDOW(dlg), true);
	if (dir)
		gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dlg), dir.c_str());
	add_filters(dlg, p.show_filter, p.filter);
	gtk_widget_show_all(dlg);
	run_later(0.01f, [dlg, p] {
		int r = gtk_dialog_run(GTK_DIALOG(dlg));
		if (r == GTK_RESPONSE_ACCEPT) {
			gchar *fn = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dlg));
			gtk_widget_destroy(dlg);
			cur_file_promise(p.try_to_ensure_extension(Path(fn)));
			g_free(fn);
		} else {
			gtk_widget_destroy(dlg);
			cur_file_promise.fail();
		}
	});
#endif
	return cur_file_promise.get_future();
}


// file selection for saving
FileFuture file_dialog_save(Window *win, const string &title, const Path &dir, const Array<string> &params) {
	cur_file_promise.reset();
	auto p = DialogParams::parse(params);
	DialogParams::STATIC_PARAMS = p;
	GtkWindow *w = get_window_save(win);
#if GTK_CHECK_VERSION(4,10,0)
	auto dlg = gtk_file_dialog_new();
	gtk_file_dialog_set_title(dlg, sys_str(title));
	if (dir)
		gtk_file_dialog_set_initial_folder(dlg, g_file_new_for_path(dir.str().c_str()));
	add_filters(dlg, p);
	gtk_file_dialog_save(dlg, w, nullptr, &on_file_dialog_save, nullptr);
#elif GTK_CHECK_VERSION(4,0,0)
	GtkWidget *dlg = gtk_file_chooser_dialog_new(sys_str(title),
												w,
												GTK_FILE_CHOOSER_ACTION_SAVE,
												_("Cancel").c_str(),	GTK_RESPONSE_CANCEL,
												_("Save").c_str(),		GTK_RESPONSE_ACCEPT,
												nullptr);
	gtk_window_set_modal(GTK_WINDOW(dlg), true);
	//gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER (dlg), TRUE);
//	gtk_file_chooser_set_current_name();
	if (dir) {
		auto path = g_file_new_for_path(dir.c_str());
		gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dlg), path, nullptr);
		//g_object_unref(path);
	}
	if (p.default_name)
		gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(dlg), sys_str(*p.default_name));
	add_filters(dlg, p.show_filter, p.filter);
	g_signal_connect(dlg, "response", G_CALLBACK(on_gtk_file_dialog_response), nullptr);
	gtk_window_present(GTK_WINDOW(dlg));
#else
	GtkWidget *dlg = gtk_file_chooser_dialog_new(sys_str(title),
												w,
												GTK_FILE_CHOOSER_ACTION_SAVE,
												"gtk-cancel",	GTK_RESPONSE_CANCEL,
												"gtk-open",		GTK_RESPONSE_ACCEPT,
												nullptr);
	gtk_window_set_modal(GTK_WINDOW(dlg), true);
	gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(dlg), TRUE);
	if (dir)
		gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dlg), dir.c_str());
	if (p.default_name)
		gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(dlg), sys_str(*p.default_name));
	add_filters(dlg, p.show_filter, p.filter);
	gtk_widget_show_all(dlg);
	run_later(0.01f, [dlg, p] {
		int r = gtk_dialog_run(GTK_DIALOG(dlg));
		if (r == GTK_RESPONSE_ACCEPT) {
			gchar *fn = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dlg));
			gtk_widget_destroy(dlg);
			cur_file_promise(p.try_to_ensure_extension(Path(fn)));
			g_free(fn);
		} else {
			gtk_widget_destroy(dlg);
			cur_file_promise.fail();
		}
	});
#endif
	return cur_file_promise.get_future();
}


static base::promise<string> cur_font_promise;

#if GTK_CHECK_VERSION(4,10,0)
static void on_select_font(GObject* o, GAsyncResult* res, gpointer user_data) {
	auto dlg = GTK_FONT_DIALOG(o);
	if (auto f = gtk_font_dialog_choose_font_finish(dlg, res, nullptr))
		cur_font_promise(pango_font_description_to_string(f));
	else
		cur_font_promise.fail();
}
#elif GTK_CHECK_VERSION(4,0,0)
static void on_gtk_font_dialog_response(GtkDialog *self, gint response_id, gpointer user_data) {
	if (response_id == GTK_RESPONSE_OK) {
		gchar *fn = gtk_font_chooser_get_font(GTK_FONT_CHOOSER(self));
		cur_font_promise(fn);
		g_free(fn);
	} else {
		cur_font_promise.fail();
	}
	gtk_window_destroy(GTK_WINDOW(self));
}
#endif

FontFuture select_font(Window *win, const string &title, const Array<string> &params) {
	cur_font_promise.reset();
	auto p = DialogParams::parse(params);
	GtkWindow *w = get_window_save(win);
#if GTK_CHECK_VERSION(4,10,0)
	auto dlg = gtk_font_dialog_new();
	gtk_font_dialog_set_title(dlg, sys_str(title));
	PangoFontDescription *f0 = nullptr;
	if (p.font)
		f0 = pango_font_description_from_string((*p.font).c_str());
	gtk_font_dialog_choose_font(dlg, w, f0, nullptr, &on_select_font, nullptr);
//	if (f0)
//		g_object_unref(f0);
#elif GTK_CHECK_VERSION(4,0,0)
	GtkWidget *dlg = gtk_font_chooser_dialog_new(sys_str(title), w);
	gtk_window_set_modal(GTK_WINDOW(dlg), true);
	if (p.font)
		gtk_font_chooser_set_font(GTK_FONT_CHOOSER(dlg), (*p.font).c_str());
	g_signal_connect(dlg, "response", G_CALLBACK(on_gtk_font_dialog_response), nullptr);
	gtk_window_present(GTK_WINDOW(dlg));
#elif GTK_CHECK_VERSION(3,0,0)
	GtkWidget *dlg = gtk_font_chooser_dialog_new(sys_str(title), w);
	gtk_window_set_modal(GTK_WINDOW(dlg), true);
	gtk_widget_show_all(dlg);
	run_later(0.01f, [dlg] {
		int r = gtk_dialog_run(GTK_DIALOG(dlg));
		if (r == GTK_RESPONSE_OK) {
			gchar *fn = gtk_font_chooser_get_font(GTK_FONT_CHOOSER(dlg));
			cur_font_promise(fn);
			g_free(fn);
		} else {
			cur_font_promise.fail();
		}
		gtk_widget_destroy(dlg);
	});
#endif
	return cur_font_promise.get_future();
}


// -> ColorButton
color color_gtk_to_user(const color &c);
color color_user_to_gtk(const color &c);
GdkRGBA color_to_gdk(const color &c);
color color_from_gdk(const GdkRGBA &gcol);

static base::promise<color> cur_color_promise;

#if GTK_CHECK_VERSION(4,10,0)
static void on_select_color(GObject* o, GAsyncResult* res, gpointer user_data) {
	auto dlg = GTK_COLOR_DIALOG(o);
	if (auto c = gtk_color_dialog_choose_rgba_finish(dlg, res, nullptr))
		cur_color_promise(color_gtk_to_user(color_from_gdk(*c)));
	else
		cur_color_promise.fail();
}
#elif GTK_CHECK_VERSION(4,0,0)
static void on_gtk_color_dialog_response(GtkDialog *self, gint response_id, gpointer user_data) {
	if (response_id == GTK_RESPONSE_OK) {
		GdkRGBA gcol;
		gtk_color_chooser_get_rgba(GTK_COLOR_CHOOSER(self), &gcol);
		cur_color_promise(color_gtk_to_user(color_from_gdk(gcol)));
	} else {
		cur_color_promise.fail();
	}
	gtk_window_destroy(GTK_WINDOW(self));
}
#endif

ColorFuture select_color(Window *win, const string &title, const color &c) {
	cur_color_promise.reset();
	GdkRGBA gcol = color_to_gdk(color_user_to_gtk(c));
	GtkWindow *w = get_window_save(win);
#if GTK_CHECK_VERSION(4,10,0)
	auto dlg = gtk_color_dialog_new();
	gtk_color_dialog_set_title(dlg, sys_str(title));
	//gtk_color_dialog_set_with_alpha(dlg, true);
	gtk_color_dialog_choose_rgba(dlg, w, &gcol, nullptr, &on_select_color, nullptr);
#elif GTK_CHECK_VERSION(4,0,0)
	auto dlg = gtk_color_chooser_dialog_new("Color", w);
	gtk_color_chooser_set_rgba(GTK_COLOR_CHOOSER(dlg), &gcol);
	g_signal_connect(dlg, "response", G_CALLBACK(on_gtk_color_dialog_response), nullptr);
	gtk_window_present(GTK_WINDOW(dlg));
#else
	auto dlg = gtk_color_chooser_dialog_new("Color", w);
	gtk_color_chooser_set_rgba(GTK_COLOR_CHOOSER(dlg), &gcol);
	gtk_widget_show_all(dlg);
	run_later(0.01f, [dlg, gcol] {
		auto gc = gcol;
		int r = gtk_dialog_run(GTK_DIALOG(dlg));
		if (r == GTK_RESPONSE_OK) {
			gtk_color_chooser_get_rgba(GTK_COLOR_CHOOSER(dlg), &gc);
			cur_color_promise(color_gtk_to_user(color_from_gdk(gc)));
		} else {
			cur_color_promise.fail();
		}
	});
	gtk_widget_destroy(dlg);
#endif
	return cur_color_promise.get_future();
}


static base::promise<bool> cur_question_promise;

#if GTK_CHECK_VERSION(4,10,0)
static void on_question_reply(GObject* o, GAsyncResult* res, gpointer user_data) {
	auto dlg = GTK_ALERT_DIALOG(o);
	int r = gtk_alert_dialog_choose_finish(dlg, res, nullptr);
	if (r == 0)
		cur_question_promise(true);
	else if (r == 1)
		cur_question_promise(false);
	else
		cur_question_promise.fail();
}
#elif GTK_CHECK_VERSION(4,0,0)
static void on_gtk_question_dialog_response(GtkDialog *self, gint response_id, gpointer user_data) {
	if (response_id == GTK_RESPONSE_YES)
		cur_question_promise(true);
	else if (response_id == GTK_RESPONSE_NO)
		cur_question_promise(false);
	else
		cur_question_promise.fail();
	gtk_window_destroy(GTK_WINDOW(self));
}
#endif

QuestionFuture question_box(Window *win, const string &title, const string &text, bool allow_cancel) {
	cur_question_promise.reset();
	GtkWindow *w = get_window_save(win);
#if GTK_CHECK_VERSION(4,10,0)
	auto dlg = gtk_alert_dialog_new("%s", sys_str(title));
	gtk_alert_dialog_set_detail(dlg, sys_str(text));
	const char* buttons[3] = {"Yes", "No", nullptr};
	const char* buttons_cancel[4] = {"Yes", "No", "Cancel", nullptr};
	if (allow_cancel)
		gtk_alert_dialog_set_buttons(dlg, buttons_cancel);
	else
		gtk_alert_dialog_set_buttons(dlg, buttons);
	gtk_alert_dialog_set_default_button(dlg, 0);
	gtk_alert_dialog_set_cancel_button(dlg, 2);
	gtk_alert_dialog_choose(dlg, w, nullptr, &on_question_reply, nullptr);
#else
	GtkWidget *dlg = gtk_message_dialog_new(w, GTK_DIALOG_MODAL, GTK_MESSAGE_QUESTION, GTK_BUTTONS_NONE, "%s", sys_str(text));
	gtk_dialog_add_button(GTK_DIALOG(dlg),  "Yes", GTK_RESPONSE_YES);
	gtk_dialog_add_button(GTK_DIALOG(dlg),  "No", GTK_RESPONSE_NO);
	if (allow_cancel)
		gtk_dialog_add_button(GTK_DIALOG(dlg),  "Cancel", GTK_RESPONSE_CANCEL);
	gtk_window_set_modal(GTK_WINDOW(dlg), true);
	//gtk_window_set_default_size(GTK_WINDOW(dlg), 300, 150);
	gtk_window_set_title(GTK_WINDOW(dlg), sys_str(title));
#if GTK_CHECK_VERSION(4,0,0)
	g_signal_connect(dlg, "response", G_CALLBACK(on_gtk_question_dialog_response), nullptr);
	gtk_window_present(GTK_WINDOW(dlg));
#else
	gtk_window_resize(GTK_WINDOW(dlg), 300, 100);
	gtk_widget_show_all(dlg);
	run_later(0.01f, [dlg] {
		gint result = gtk_dialog_run(GTK_DIALOG(dlg));
		gtk_widget_destroy(dlg);
		if (result == GTK_RESPONSE_YES)
			cur_question_promise(true);
		else if (result == GTK_RESPONSE_NO)
			cur_question_promise(false);
		else
			cur_question_promise.fail();
	});
#endif
#endif
	return cur_question_promise.get_future();
}


#if GTK_CHECK_VERSION(4,10,0)
static void on_message_reply(GObject* o, GAsyncResult* res, gpointer user_data) {
	auto dlg = GTK_ALERT_DIALOG(o);
	[[maybe_unused]] int r = gtk_alert_dialog_choose_finish(dlg, res, nullptr);
}
#else
void on_gtk_message_dialog_response(GtkDialog *self, gint response_id, gpointer user_data) {
	gtk_window_destroy(GTK_WINDOW(self));
}
#endif

void info_box(Window *win,const string &title,const string &text) {
	GtkWindow *w = get_window_save(win);
#if GTK_CHECK_VERSION(4,10,0)
	auto dlg = gtk_alert_dialog_new("%s", sys_str(title));
	gtk_alert_dialog_set_detail(dlg, sys_str(text));
	const char* buttons[2] = {"Ok", nullptr};
	gtk_alert_dialog_set_buttons(dlg, buttons);
	gtk_alert_dialog_set_default_button(dlg, 0);
	gtk_alert_dialog_choose(dlg, w, nullptr, &on_message_reply, nullptr);
#else
	GtkWidget *dlg = gtk_message_dialog_new(w, GTK_DIALOG_MODAL, GTK_MESSAGE_INFO, GTK_BUTTONS_OK, "%s", sys_str(text));
	gtk_window_set_modal(GTK_WINDOW(dlg), true);
	//gtk_window_set_default_size(GTK_WINDOW(dlg), 300, 100);
	//gtk_window_resize(GTK_WINDOW(dlg), 300, 100);
	gtk_window_set_title(GTK_WINDOW(dlg), sys_str(title));
#if GTK_CHECK_VERSION(4,0,0)
	g_signal_connect(dlg, "response", G_CALLBACK(on_gtk_message_dialog_response), nullptr);
	gtk_window_present(GTK_WINDOW(dlg));
#else
	gtk_widget_show_all(dlg);
	gtk_dialog_run(GTK_DIALOG(dlg));
	gtk_widget_destroy(dlg);
#endif
#endif
}

void error_box(Window *win,const string &title,const string &text) {
	GtkWindow *w = get_window_save(win);
#if GTK_CHECK_VERSION(4,10,0)
	auto dlg = gtk_alert_dialog_new("%s", sys_str(title)); // \u26A0
	gtk_alert_dialog_set_detail(dlg, sys_str(text));
	const char* buttons[2] = {"Ok", nullptr};
	gtk_alert_dialog_set_buttons(dlg, buttons);
	gtk_alert_dialog_set_default_button(dlg, 0);
	gtk_alert_dialog_choose(dlg, w, nullptr, &on_message_reply, nullptr);
#else
	GtkWidget *dlg = gtk_message_dialog_new(w, GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, "%s", sys_str(text));
	gtk_window_set_modal(GTK_WINDOW(dlg), true);
	//gtk_window_set_default_size(GTK_WINDOW(dlg), 300, 100);
	gtk_window_set_title(GTK_WINDOW(dlg), sys_str(title));
#if GTK_CHECK_VERSION(4,0,0)
	g_signal_connect(dlg, "response", G_CALLBACK(on_gtk_message_dialog_response), nullptr);
	gtk_window_present(GTK_WINDOW(dlg));
#else
	gtk_widget_show_all(dlg);
	gtk_dialog_run(GTK_DIALOG(dlg));
	gtk_widget_destroy(dlg);
#endif
#endif
}

static Array<char*> sa2ca_nt(const Array<string> &a) {
	Array<char*> _a_;
	for (string &s: a)
		if (a.num > 0) {
			char *p = new char[s.num + 1];
			strcpy(p, s.c_str());
			_a_.add(p);
		}
	_a_.add(nullptr);
	return _a_;
}

#if !GTK_CHECK_VERSION(4,0,0)
static void ca_free(Array<char*> &a) {
	for (char *aa: a)
		if (aa)
			delete[] aa;
	a.clear();
}
#endif




void on_gtk_about_dialog_response(GtkDialog *self, gint response_id, gpointer user_data) {
	gtk_window_destroy(GTK_WINDOW(self));
}

void about_box(Window *win) {
	// load license
	if (Application::get_property("license") == "")
		if (os::fs::exists(os::app::directory_static | "license_small.txt"))
			Application::set_property("license", os::fs::read_text(os::app::directory_static | "license_small.txt"));

	// author list
	auto authors = sa2ca_nt(Application::get_property("author").explode(";"));
	auto artists = sa2ca_nt(Application::get_property("artist").explode(";"));
	auto designers = sa2ca_nt(Application::get_property("designer").explode(";"));
	auto documenters = sa2ca_nt(Application::get_property("documenter").explode(";"));
	auto translators = sa2ca_nt(Application::get_property("translator").explode(";"));

	[[maybe_unused]] GError *error = nullptr;
	auto dlg = gtk_about_dialog_new();
	gtk_about_dialog_set_program_name(GTK_ABOUT_DIALOG(dlg), Application::get_property("name").c_str());
	gtk_about_dialog_set_version(GTK_ABOUT_DIALOG(dlg), Application::get_property("version").c_str());
	gtk_about_dialog_set_comments(GTK_ABOUT_DIALOG(dlg), Application::get_property("comment").c_str());
	gtk_about_dialog_set_license(GTK_ABOUT_DIALOG(dlg), Application::get_property("license").c_str());
	gtk_about_dialog_set_website(GTK_ABOUT_DIALOG(dlg), Application::get_property("website").c_str());
#if GTK_CHECK_VERSION(4,0,0)

	auto icon_theme = gtk_icon_theme_get_for_display(gdk_display_get_default());
	auto icon = gtk_icon_theme_lookup_icon(icon_theme,
									   "tsunami", // icon name
									   nullptr,
									   128, // icon size
									   1,  // scale
									   GTK_TEXT_DIR_NONE,
									   GTK_ICON_LOOKUP_FORCE_REGULAR);  // flags);

	gtk_about_dialog_set_logo(GTK_ABOUT_DIALOG(dlg), GDK_PAINTABLE(icon));

	//msg_write("ABOUT..." + Application::get_property("logo"));
	/*auto *_logo = gdk_texture_new_from_file(g_file_new_for_path(Application::get_property("logo").c_str()), &error);
	if (error)
		msg_error(error->message);
	gtk_about_dialog_set_logo(GTK_ABOUT_DIALOG(dlg), GDK_PAINTABLE(_logo));*/


	/*auto icon_theme = gtk_icon_theme_get_for_display(gtk_widget_get_display(dlg));
	gtk_icon_theme_add_search_path(icon_theme, str(Application::directory_static | "icons").c_str());

	string icon = Application::get_property("icon");
	auto p = gtk_icon_theme_lookup_icon(icon_theme, icon.c_str(), nullptr, 128, 1, GTK_TEXT_DIR_NONE, GTK_ICON_LOOKUP_FORCE_REGULAR);

	gtk_about_dialog_set_logo(GTK_ABOUT_DIALOG(dlg), GDK_PAINTABLE(p));*/
#else
	auto *_logo = gdk_pixbuf_new_from_file(Application::get_property("logo").c_str(), &error);
	gtk_about_dialog_set_logo(GTK_ABOUT_DIALOG(dlg), _logo);
#endif
	gtk_about_dialog_set_authors(GTK_ABOUT_DIALOG(dlg), (const char**)authors.data);
	if (designers.num > 1)
		gtk_about_dialog_add_credit_section(GTK_ABOUT_DIALOG(dlg), "Designed by", (const char**)designers.data);
	if (documenters.num > 1)
		gtk_about_dialog_set_documenters(GTK_ABOUT_DIALOG(dlg), (const char**)documenters.data);
	//if (translators.num > 0)
	//gtk_about_dialog_set_translator_credits(GTK_ABOUT_DIALOG(dlg), "no one :P");//(const char**)translators.data);
	gtk_window_set_transient_for(GTK_WINDOW(dlg), GTK_WINDOW(win->window));


#if GTK_CHECK_VERSION(4,0,0)
	gtk_window_set_modal(GTK_WINDOW(dlg), true);
//	g_signal_connect(dlg, "response", G_CALLBACK(on_gtk_about_dialog_response), nullptr);
	gtk_window_present(GTK_WINDOW(dlg));
#else
	gtk_dialog_run(GTK_DIALOG(dlg));
	gtk_widget_destroy(dlg);

	ca_free(authors);
	ca_free(artists);
	ca_free(designers);
	ca_free(documenters);
	ca_free(translators);
#endif
}

};

