#include "hui.h"
#ifdef HUI_API_GTK

#include "../os/file.h"
#include "../os/filesystem.h"

namespace hui
{


struct DialogParams {
	string filter, show_filter;
	string font;
	string default_name;
	string default_extension;
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

	Path try_to_ensure_extension(const Path &filename) {
		if (filename.extension() == "" and default_extension != "")
			return filename.with("." + default_extension);

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

static FileDialogCallback cur_file_callback;
void on_gtk_file_dialog_response(GtkDialog *self, gint response_id, gpointer user_data) {
	if (response_id == GTK_RESPONSE_ACCEPT) {
		auto fn = gtk_file_chooser_get_file(GTK_FILE_CHOOSER(self));
		gtk_window_destroy(GTK_WINDOW(self));
		cur_file_callback(DialogParams::STATIC_PARAMS.try_to_ensure_extension(g_file_get_path(fn)));
	} else {
		gtk_window_destroy(GTK_WINDOW(self));
		cur_file_callback("");
	}
}

// choose a directory (<dir> will be selected initially)
void file_dialog_dir(Window *win, const string &title, const Path &dir, const Array<string> &params, const FileDialogCallback &cb) {
	auto p = DialogParams::parse(params);
#if GTK_CHECK_VERSION(4,0,0)
	GtkWindow *w = get_window_save(win);
	GtkWidget* dlg = gtk_file_chooser_dialog_new(sys_str(title),
												w,
												GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER,
												_("Cancel").c_str(),	GTK_RESPONSE_CANCEL,
												_("Open").c_str(),		GTK_RESPONSE_ACCEPT,
												nullptr);
	if (dir)
		gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dlg), g_file_new_for_path(dir.str().c_str()), nullptr);
	cur_file_callback = cb;
	g_signal_connect(dlg, "response", G_CALLBACK(on_gtk_file_dialog_response), nullptr);
	gtk_window_present(GTK_WINDOW(dlg));
#else
	GtkWindow *w = get_window_save(win);
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
		cb(fn);
	} else {
		gtk_widget_destroy(dlg);
	}
#endif
}

void add_filters(GtkWidget *dlg, const string &show_filter, const string &filter) {
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

// file selection for opening (filter should look like "*.txt")
void file_dialog_open(Window *win, const string &title, const Path &dir, const Array<string> &params, const FileDialogCallback &cb) {
	auto p = DialogParams::parse(params);
	DialogParams::STATIC_PARAMS = p;
#if GTK_CHECK_VERSION(4,0,0)
	GtkWindow *w = get_window_save(win);
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
	cur_file_callback = cb;
	g_signal_connect(dlg, "response", G_CALLBACK(on_gtk_file_dialog_response), nullptr);
	gtk_window_present(GTK_WINDOW(dlg));

#else
	GtkWindow *w = get_window_save(win);
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
	int r = gtk_dialog_run(GTK_DIALOG(dlg));
	if (r == GTK_RESPONSE_ACCEPT) {
		gchar *fn = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dlg));
		gtk_widget_destroy(dlg);
		cb(p.try_to_ensure_extension(Path(fn)));
		g_free(fn);
	} else {
		gtk_widget_destroy(dlg);
		cb("");
	}
#endif
}


// file selection for saving
void file_dialog_save(Window *win, const string &title, const Path &dir, const Array<string> &params, const FileDialogCallback &cb) {
	auto p = DialogParams::parse(params);
	DialogParams::STATIC_PARAMS = p;
#if GTK_CHECK_VERSION(4,0,0)
	GtkWindow *w = get_window_save(win);
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
	if (p.default_name.num > 0)
		gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(dlg), sys_str(p.default_name));
	add_filters(dlg, p.show_filter, p.filter);
	cur_file_callback = cb;
	g_signal_connect(dlg, "response", G_CALLBACK(on_gtk_file_dialog_response), nullptr);
	gtk_window_present(GTK_WINDOW(dlg));

#else

	GtkWindow *w = get_window_save(win);
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
	if (p.default_name.num > 0)
		gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(dlg), sys_str(p.default_name));
	add_filters(dlg, p.show_filter, p.filter);
	gtk_widget_show_all(dlg);
	int r = gtk_dialog_run(GTK_DIALOG(dlg));
	if (r == GTK_RESPONSE_ACCEPT) {
		gchar *fn = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dlg));
		gtk_widget_destroy(dlg);
		cb(p.try_to_ensure_extension(Path(fn)));
		g_free(fn);
	} else {
		gtk_widget_destroy(dlg);
		cb("");
	}
#endif
}


static FontDialogCallback cur_font_callback;
void on_gtk_font_dialog_response(GtkDialog *self, gint response_id, gpointer user_data) {
	if (response_id == GTK_RESPONSE_OK) {
		gchar *fn = gtk_font_chooser_get_font(GTK_FONT_CHOOSER(self));
		cur_font_callback(fn);
		g_free(fn);
	}
	gtk_window_destroy(GTK_WINDOW(self));
}

void select_font(Window *win, const string &title, const Array<string> &params, const FontDialogCallback &cb) {
	auto p = DialogParams::parse(params);
#if GTK_CHECK_VERSION(4,0,0)
	GtkWindow *w = get_window_save(win);
	GtkWidget *dlg = gtk_font_chooser_dialog_new(sys_str(title), w);
	gtk_window_set_modal(GTK_WINDOW(dlg), true);
	gtk_font_chooser_set_font(GTK_FONT_CHOOSER(dlg), p.font.c_str());

	cur_font_callback = cb;
	g_signal_connect(dlg, "response", G_CALLBACK(on_gtk_font_dialog_response), nullptr);
	gtk_window_present(GTK_WINDOW(dlg));
#elif GTK_CHECK_VERSION(3,0,0)
	GtkWindow *w = get_window_save(win);
	GtkWidget *dlg = gtk_font_chooser_dialog_new(sys_str(title), w);
	gtk_window_set_modal(GTK_WINDOW(dlg), true);
	gtk_widget_show_all(dlg);
	int r = gtk_dialog_run(GTK_DIALOG(dlg));
	if (r == GTK_RESPONSE_OK) {
		gchar *fn = gtk_font_chooser_get_font(GTK_FONT_CHOOSER(dlg));
		cb(fn);
		g_free(fn);
	}
	gtk_widget_destroy(dlg);
#endif
}


static ColorDialogCallback cur_color_callback;
void on_gtk_color_dialog_response(GtkDialog *self, gint response_id, gpointer user_data) {
	if (response_id == GTK_RESPONSE_OK) {
		GdkRGBA gcol;
		gtk_color_chooser_get_rgba(GTK_COLOR_CHOOSER(self), &gcol);
		color c;
		c.r = (float)gcol.red;
		c.g = (float)gcol.green;
		c.b = (float)gcol.blue;
		c.a = (float)gcol.alpha;
		cur_color_callback(c);
	}
	gtk_window_destroy(GTK_WINDOW(self));
}

void select_color(Window *win, const string &title, const color &c, const ColorDialogCallback &cb) {
#if GTK_CHECK_VERSION(4,0,0)

	GtkWindow *w = get_window_save(win);
	auto dlg = gtk_color_chooser_dialog_new("Color", w);

	GdkRGBA gcol;
	gcol.red = c.r;
	gcol.green = c.g;
	gcol.blue = c.b;
	gcol.alpha = c.a;
	gtk_color_chooser_set_rgba(GTK_COLOR_CHOOSER(dlg), &gcol);

	cur_color_callback = cb;
	g_signal_connect(dlg, "response", G_CALLBACK(on_gtk_color_dialog_response), nullptr);
	gtk_window_present(GTK_WINDOW(dlg));
#else
	GtkWindow *w = get_window_save(win);
	auto dlg = gtk_color_chooser_dialog_new("Color", w);

	GdkRGBA gcol;
	gcol.red = c.r;
	gcol.green = c.g;
	gcol.blue = c.b;
	gcol.alpha = c.a;
	gtk_color_chooser_set_rgba(GTK_COLOR_CHOOSER(dlg), &gcol);

	gtk_widget_show_all(dlg);
	int r = gtk_dialog_run(GTK_DIALOG(dlg));
	if (r == GTK_RESPONSE_OK) {
		gtk_color_chooser_get_rgba(GTK_COLOR_CHOOSER(dlg), &gcol);
		color Color;
		Color.r = (float)gcol.red;
		Color.g = (float)gcol.green;
		Color.b = (float)gcol.blue;
		Color.a = (float)gcol.alpha;
		cb(Color);
	}
	gtk_widget_destroy(dlg);
#endif
}


static QuestionDialogCallback cur_question_callback;
void on_gtk_question_dialog_response(GtkDialog *self, gint response_id, gpointer user_data) {
	if (response_id == GTK_RESPONSE_YES)
		cur_question_callback("hui:yes");
	else if (response_id == GTK_RESPONSE_NO)
		cur_question_callback("hui:no");
	else
		cur_question_callback("hui:cancel");
	gtk_window_destroy(GTK_WINDOW(self));
}

void question_box(Window *win, const string &title, const string &text, const QuestionDialogCallback &cb, bool allow_cancel) {
	GtkWindow *w = get_window_save(win);
	GtkWidget *dlg = gtk_message_dialog_new(w, GTK_DIALOG_MODAL, GTK_MESSAGE_QUESTION, GTK_BUTTONS_YES_NO, "%s", sys_str(text));
	gtk_window_set_modal(GTK_WINDOW(dlg), true);
	//gtk_window_set_default_size(GTK_WINDOW(dlg), 300, 150);
	gtk_window_set_title(GTK_WINDOW(dlg), sys_str(title));
#if GTK_CHECK_VERSION(4,0,0)
	cur_question_callback = cb;
	g_signal_connect(dlg, "response", G_CALLBACK(on_gtk_question_dialog_response), nullptr);
	gtk_window_present(GTK_WINDOW(dlg));
#else
	gtk_window_resize(GTK_WINDOW(dlg), 300, 100);
	gtk_widget_show_all(dlg);
	gint result = gtk_dialog_run(GTK_DIALOG(dlg));
	gtk_widget_destroy(dlg);
	if (result == GTK_RESPONSE_YES)
		cb("hui:yes");
	else if (result == GTK_RESPONSE_NO)
		cb("hui:no");
	else
		cb("hui:cancel");
#endif
}


void on_gtk_message_dialog_response(GtkDialog *self, gint response_id, gpointer user_data) {
	gtk_window_destroy(GTK_WINDOW(self));
}

void info_box(Window *win,const string &title,const string &text) {
	GtkWindow *w = get_window_save(win);
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
}

void error_box(Window *win,const string &title,const string &text) {
	GtkWindow *w = get_window_save(win);
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

static void ca_free(Array<char*> &a) {
	for (char *aa: a)
		if (aa)
			delete[] aa;
	a.clear();
}




void on_gtk_about_dialog_response(GtkDialog *self, gint response_id, gpointer user_data) {
	gtk_window_destroy(GTK_WINDOW(self));
}

void about_box(Window *win) {
	// load license
	if (Application::get_property("license") == "")
		if (file_exists(Application::directory_static << "license_small.txt"))
			Application::set_property("license", file_read_text(Application::directory_static << "license_small.txt"));

	// author list
	auto authors = sa2ca_nt(Application::get_property("author").explode(";"));
	auto artists = sa2ca_nt(Application::get_property("artist").explode(";"));
	auto designers = sa2ca_nt(Application::get_property("designer").explode(";"));
	auto documenters = sa2ca_nt(Application::get_property("documenter").explode(";"));
	auto translators = sa2ca_nt(Application::get_property("translator").explode(";"));

	GError *error = nullptr;
	auto dlg = gtk_about_dialog_new();
	gtk_about_dialog_set_program_name(GTK_ABOUT_DIALOG(dlg), Application::get_property("name").c_str());
	gtk_about_dialog_set_version(GTK_ABOUT_DIALOG(dlg), Application::get_property("version").c_str());
	gtk_about_dialog_set_comments(GTK_ABOUT_DIALOG(dlg), Application::get_property("comment").c_str());
	gtk_about_dialog_set_license(GTK_ABOUT_DIALOG(dlg), Application::get_property("license").c_str());
	gtk_about_dialog_set_website(GTK_ABOUT_DIALOG(dlg), Application::get_property("website").c_str());
#if GTK_CHECK_VERSION(4,0,0)
	auto *_logo = gdk_texture_new_from_file(g_file_new_for_path(Application::get_property("logo").c_str()), &error);
	gtk_about_dialog_set_logo(GTK_ABOUT_DIALOG(dlg), GDK_PAINTABLE(_logo));
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

#endif
