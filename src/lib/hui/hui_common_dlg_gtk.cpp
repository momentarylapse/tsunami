#include "hui.h"
#ifdef HUI_API_GTK


static GtkWindow *get_window_save(CHuiWindow *win)
{
	_HuiMakeUsable_();
	return win ? GTK_WINDOW(win->window) : NULL;
}

// choose a directory (<dir> will be selected initially)
bool HuiFileDialogDir(CHuiWindow *win, const string &title, const string &dir/*, const string &root_dir*/)
{
	GtkWindow *w = get_window_save(win);
	GtkWidget* dlg=gtk_file_chooser_dialog_new(	sys_str(title),
												w,
												GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER,
												GTK_STOCK_CANCEL,	GTK_RESPONSE_CANCEL,
												GTK_STOCK_OPEN,		GTK_RESPONSE_ACCEPT,
												NULL);
	gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dlg), sys_str_f(dir));
	int r = gtk_dialog_run(GTK_DIALOG(dlg));
	if (r == GTK_RESPONSE_ACCEPT){
		HuiFilename = gtk_file_chooser_get_current_folder(GTK_FILE_CHOOSER (dlg));
		HuiFilename.dir_ensure_ending();
	}
	gtk_widget_destroy(dlg);
	return (r == GTK_RESPONSE_ACCEPT);
}

void add_filters(GtkWidget *dlg, const string &show_filter, const string &filter)
{
	GtkFileFilter *gtk_filter = gtk_file_filter_new();
	gtk_file_filter_set_name(gtk_filter, sys_str(show_filter));
	Array<string> filters = filter.explode(";");
	foreach(string &f, filters)
		gtk_file_filter_add_pattern(gtk_filter, sys_str(f));
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dlg), gtk_filter);
}

// file selection for opening (filter should look like "*.txt")
bool HuiFileDialogOpen(CHuiWindow *win,const string &title,const string &dir,const string &show_filter,const string &filter)
{
	msg_db_r("HuiFileDialogOpen",1);
	GtkWindow *w = get_window_save(win);
	msg_db_m("dialog_new",1);
	GtkWidget *dlg=gtk_file_chooser_dialog_new(	sys_str(title),
												w,
												GTK_FILE_CHOOSER_ACTION_OPEN,
												GTK_STOCK_CANCEL,	GTK_RESPONSE_CANCEL,
												GTK_STOCK_OPEN,		GTK_RESPONSE_ACCEPT,
												NULL);
	gtk_window_set_modal(GTK_WINDOW(dlg), true);
	gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dlg), sys_str_f(dir));
	add_filters(dlg, show_filter, filter);
	gtk_widget_show_all(dlg);
	msg_db_m("dialog_run",1);
	int r = gtk_dialog_run(GTK_DIALOG(dlg));
	msg_db_m("ok",1);
	if (r == GTK_RESPONSE_ACCEPT)
		HuiFilename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dlg));
	gtk_widget_destroy(dlg);
	msg_db_l(1);
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
bool HuiFileDialogSave(CHuiWindow *win,const string &title,const string &dir,const string &show_filter,const string &filter)
{
	GtkWindow *w = win ? GTK_WINDOW(win->window) : NULL;
	GtkWidget* dlg=gtk_file_chooser_dialog_new(	sys_str(title),
												w,
												GTK_FILE_CHOOSER_ACTION_SAVE,
												GTK_STOCK_CANCEL,	GTK_RESPONSE_CANCEL,
												GTK_STOCK_SAVE,		GTK_RESPONSE_ACCEPT,
												NULL);
	gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER (dlg), TRUE);
	gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dlg), sys_str_f(dir));
	add_filters(dlg, show_filter, filter);
	int r = gtk_dialog_run(GTK_DIALOG(dlg));
	if (r == GTK_RESPONSE_ACCEPT){
		HuiFilename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dlg));
		try_to_ensure_extension(HuiFilename, filter);
		// TODO
	}
	gtk_widget_destroy (dlg);
	return (r==GTK_RESPONSE_ACCEPT);
}

bool HuiSelectColor(CHuiWindow *win,int r,int g,int b)
{
	msg_todo("HuiSelectColor (GTK)");
	return false;
}

string HuiQuestionBox(CHuiWindow *win,const string &title,const string &text,bool allow_cancel)
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

void HuiInfoBox(CHuiWindow *win,const string &title,const string &text)
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

void HuiErrorBox(CHuiWindow *win,const string &title,const string &text)
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

void HuiAboutBox(CHuiWindow *win)
{
	Array<char*> _a_;
	foreach(string &author, HuiPropAuthors){
		char *p = new char[author.num + 1];
		strcpy(p, author.c_str());
		_a_.add(p);
	}
	_a_.add(NULL);
	GError *error = NULL;
	GdkPixbuf *_logo = gdk_pixbuf_new_from_file(HuiPropLogo.c_str(), &error);
	gtk_show_about_dialog(NULL,
		"program-name", HuiPropName.c_str(),
		"website", HuiPropWebsite.c_str(),
		"version", HuiPropVersion.c_str(),
		"license", HuiPropLicense.c_str(),
		"comments", sys_str(HuiPropComment.c_str()),
		"authors", _a_.data,
		"logo", _logo,
		"copyright", HuiPropCopyright.c_str(),
		NULL);

	foreach(char *aa, _a_)
		delete(aa);
}


#endif
