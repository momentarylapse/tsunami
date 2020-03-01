#include "hui.h"
#ifdef HUI_API_GTK

namespace hui
{


string file_dialog_default;


static GtkWindow *get_window_save(Window *win)
{
	_MakeUsable_();
	return win ? GTK_WINDOW(win->window) : nullptr;
}

// choose a directory (<dir> will be selected initially)
bool FileDialogDir(Window *win, const string &title, const string &dir/*, const string &root_dir*/)
{
	GtkWindow *w = get_window_save(win);
	GtkWidget* dlg=gtk_file_chooser_dialog_new(	sys_str(title),
												w,
												GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER,
												"gtk-cancel",	GTK_RESPONSE_CANCEL,
												"gtk-open",		GTK_RESPONSE_ACCEPT,
												nullptr);
	gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dlg), sys_str_f(dir));
	int r = gtk_dialog_run(GTK_DIALOG(dlg));
	if (r == GTK_RESPONSE_ACCEPT){
		Filename = gtk_file_chooser_get_current_folder(GTK_FILE_CHOOSER (dlg));
		Filename.dir_ensure_ending();
	}
	gtk_widget_destroy(dlg);
	return (r == GTK_RESPONSE_ACCEPT);
}

void add_filters(GtkWidget *dlg, const string &show_filter, const string &filter)
{
	Array<string> show_filter_list = show_filter.explode("|");
	Array<string> filter_list = filter.explode("|");
	for (int i=0; i<min(show_filter_list.num, filter_list.num); i++){
		GtkFileFilter *gtk_filter = gtk_file_filter_new();
		gtk_file_filter_set_name(gtk_filter, sys_str(show_filter_list[i]));
		Array<string> filters = filter_list[i].explode(";");
		for (string &f : filters)
			gtk_file_filter_add_pattern(gtk_filter, sys_str(f));
		gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dlg), gtk_filter);
	}
}

// file selection for opening (filter should look like "*.txt")
bool FileDialogOpen(Window *win,const string &title,const string &dir,const string &show_filter,const string &filter)
{
	GtkWindow *w = get_window_save(win);
	GtkWidget *dlg=gtk_file_chooser_dialog_new(	sys_str(title),
												w,
												GTK_FILE_CHOOSER_ACTION_OPEN,
												"gtk-cancel",	GTK_RESPONSE_CANCEL,
												"gtk-open",		GTK_RESPONSE_ACCEPT,
												nullptr);
	gtk_window_set_modal(GTK_WINDOW(dlg), true);
	gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dlg), sys_str_f(dir));
	add_filters(dlg, show_filter, filter);
	gtk_widget_show_all(dlg);
	int r = gtk_dialog_run(GTK_DIALOG(dlg));
	if (r == GTK_RESPONSE_ACCEPT){
		gchar *fn = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dlg));
		Filename = fn;
		g_free(fn);
	}
	gtk_widget_destroy(dlg);
	return (r == GTK_RESPONSE_ACCEPT);
}

static void try_to_ensure_extension(string &filename, const string &filter)
{
	if (filter.find(".") < 0)
		return;
	// multiple choices -> ignore
	if (filter.find(";") >= 0)
		return;
	string filter_ext = filter.extension(); // "*.ext"
	// not the wanted extension -> add
	if (filename.extension() != filter_ext)
		filename += "." + filter_ext;
}


// file selection for saving
bool FileDialogSave(Window *win,const string &title,const string &dir,const string &show_filter,const string &filter)
{
	GtkWindow *w = win ? GTK_WINDOW(win->window) : nullptr;
	GtkWidget* dlg=gtk_file_chooser_dialog_new(	sys_str(title),
												w,
												GTK_FILE_CHOOSER_ACTION_SAVE,
												"gtk-cancel",	GTK_RESPONSE_CANCEL,
												"gtk-save",		GTK_RESPONSE_ACCEPT,
												nullptr);
	gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER (dlg), TRUE);
	gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dlg), sys_str_f(dir));
	if (file_dialog_default.num > 0)
		gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(dlg), sys_str(file_dialog_default));
	add_filters(dlg, show_filter, filter);
	int r = gtk_dialog_run(GTK_DIALOG(dlg));
	if (r == GTK_RESPONSE_ACCEPT){
		gchar *fn = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dlg));
		Filename = fn;
		g_free(fn);
		try_to_ensure_extension(Filename, filter);
		// TODO
	}
	gtk_widget_destroy (dlg);
	return (r==GTK_RESPONSE_ACCEPT);
}

bool SelectFont(Window *win, const string &title)
{
#if GTK_CHECK_VERSION(3,0,0)
	GtkWindow *w = get_window_save(win);
	GtkWidget *dlg = gtk_font_chooser_dialog_new(sys_str(title), w);
	gtk_window_set_modal(GTK_WINDOW(dlg), true);
	gtk_widget_show_all(dlg);
	int r = gtk_dialog_run(GTK_DIALOG(dlg));
	if (r == GTK_RESPONSE_OK){
		gchar *fn = gtk_font_chooser_get_font(GTK_FONT_CHOOSER(dlg));
		Fontname = fn;
		g_free(fn);
	}
	gtk_widget_destroy(dlg);
	return (r == GTK_RESPONSE_OK);
#endif
	return false;
}

bool SelectColor(Window *win, const color &col) {
	GtkWindow *w = get_window_save(win);
	auto dlg = gtk_color_chooser_dialog_new("Color", w);

	GdkRGBA gcol;
	gcol.red = col.r;
	gcol.green = col.g;
	gcol.blue = col.b;
	gcol.alpha = col.a;
	gtk_color_chooser_set_rgba(GTK_COLOR_CHOOSER(dlg), &gcol);

	gtk_widget_show_all(dlg);
	int r = gtk_dialog_run(GTK_DIALOG(dlg));
	if (r == GTK_RESPONSE_OK){
		gtk_color_chooser_get_rgba(GTK_COLOR_CHOOSER(dlg), &gcol);
		Color.r = (float)gcol.red;
		Color.g = (float)gcol.green;
		Color.b = (float)gcol.blue;
		Color.a = (float)gcol.alpha;
	}
	gtk_widget_destroy(dlg);
	return (r == GTK_RESPONSE_OK);
}

string QuestionBox(Window *win,const string &title,const string &text,bool allow_cancel)
{
	GtkWindow *w = get_window_save(win);
	GtkWidget *dlg = gtk_message_dialog_new(w, GTK_DIALOG_MODAL, GTK_MESSAGE_QUESTION, GTK_BUTTONS_YES_NO, "%s", sys_str(text));
	gtk_window_set_modal(GTK_WINDOW(dlg), true);
	gtk_window_resize(GTK_WINDOW(dlg), 300, 100);
	gtk_window_set_title(GTK_WINDOW(dlg), sys_str(title));
	gtk_widget_show_all(dlg);
	gint result = gtk_dialog_run(GTK_DIALOG(dlg));
	gtk_widget_destroy(dlg);
	switch (result){
		case GTK_RESPONSE_YES:
			return "hui:yes";
		case GTK_RESPONSE_NO:
			return "hui:no";
    }
	return "hui:cancel";
}

void InfoBox(Window *win,const string &title,const string &text)
{
	GtkWindow *w = get_window_save(win);
	GtkWidget *dlg = gtk_message_dialog_new(w, GTK_DIALOG_MODAL, GTK_MESSAGE_INFO, GTK_BUTTONS_OK, "%s", sys_str(text));
	gtk_window_set_modal(GTK_WINDOW(dlg), true);
	gtk_window_resize(GTK_WINDOW(dlg), 300, 100);
	gtk_window_set_title(GTK_WINDOW(dlg), sys_str(title));
	gtk_widget_show_all(dlg);
	gtk_dialog_run(GTK_DIALOG(dlg));
	gtk_widget_destroy(dlg);
}

void ErrorBox(Window *win,const string &title,const string &text)
{
	GtkWindow *w = get_window_save(win);
	GtkWidget *dlg=gtk_message_dialog_new(w, GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, "%s", sys_str(text));
	gtk_window_set_modal(GTK_WINDOW(dlg), true);
	gtk_window_resize(GTK_WINDOW(dlg), 300, 100);
	gtk_window_set_title(GTK_WINDOW(dlg), sys_str(title));
	gtk_widget_show_all(dlg);
	gtk_dialog_run(GTK_DIALOG(dlg));
	gtk_widget_destroy(dlg);
}

void AboutBox(Window *win)
{
	// load license
	if (Application::get_property("license") == "")
		if (file_test_existence(Application::directory_static + "license_small.txt"))
			Application::set_property("license", FileRead(Application::directory_static + "license_small.txt"));

	// author list
	Array<char*> _a_;
	Array<string> authors = Application::get_property("author").explode(";");
	for (string &author : authors){
		char *p = new char[author.num + 1];
		strcpy(p, author.c_str());
		_a_.add(p);
	}
	_a_.add(nullptr);

	GError *error = nullptr;
	GdkPixbuf *_logo = gdk_pixbuf_new_from_file(Application::get_property("logo").c_str(), &error);
	gtk_show_about_dialog(get_window_save(win),
		"program-name", Application::get_property("name").c_str(),
		"website", Application::get_property("website").c_str(),
		"version", Application::get_property("version").c_str(),
		"license", Application::get_property("license").c_str(),
		"comments", sys_str(Application::get_property("comment").c_str()),
		"authors", _a_.data,
		"logo", _logo,
		"copyright", Application::get_property("copyright").c_str(),
		nullptr);

	for (char *aa : _a_)
		delete(aa);
}

};

#endif
