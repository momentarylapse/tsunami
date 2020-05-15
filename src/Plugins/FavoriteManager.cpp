/*
 * FavoriteManager.cpp
 *
 *  Created on: 13.04.2014
 *      Author: michi
 */

#include "FavoriteManager.h"
#include "../Module/Module.h"
#include "../lib/file/file.h"
#include "../lib/hui/hui.h"
#include "../Tsunami.h"
#include "../Session.h"

const string FavoriteManager::DEFAULT_NAME = ":def:";

FavoriteManager::FavoriteManager() {
	loaded = false;
}

FavoriteManager::~FavoriteManager() {
}

void FavoriteManager::load_from_file(const string &filename, bool read_only, Session *session)
{
	if (!file_test_existence(filename))
		return;
	try {
		File *f = FileOpenText(filename);
		int n = f->read_int();
		for (int i=0; i<n; i++) {
			Favorite ff;
			string type = f->read_str();
			ff.type = Module::type_from_name(type);
			ff.config_name = f->read_str();
			ff.name = f->read_str();
			ff.options = f->read_str();
			ff.read_only = read_only;
			set(ff);
		}
		delete(f);
	} catch(Exception &e) {
		session->e(e.message());
	}
}

void FavoriteManager::load(Session *session) {
	load_from_file(tsunami->directory_static + "favorites_demo.txt", true, session);
	load_from_file(tsunami->directory + "favorites.txt", false, session);
	loaded = true;
}

void FavoriteManager::save(Session *session) {
	try {
		File *f = FileCreateText(tsunami->directory + "favorites.txt");
		f->write_int(favorites.num);
		for (Favorite &ff: favorites) {
			f->write_str(Module::type_to_name(ff.type));
			f->write_str(ff.config_name);
			f->write_str(ff.name);
			f->write_str(ff.options);
		}
		delete(f);
	} catch(Exception &e) {
		session->e(e.message());
	}
}

Array<string> FavoriteManager::get_list(Module *c) {
	if (!loaded)
		load(c->session);
	Array<string> r;
	for (Favorite &f: favorites) {
		if ((f.type == c->module_type) and (f.config_name == c->module_subtype))
			r.add(f.name);
	}
	return r;
}

void FavoriteManager::apply(Module *c, const string &name) {
	c->reset_config();
	if (name == DEFAULT_NAME)
		return;
	if (!loaded)
		load(c->session);
	for (Favorite &f: favorites) {
		if ((f.type == c->module_type) and (f.config_name == c->module_subtype) and (f.name == name))
			c->config_from_string(f.options);
	}
}

void FavoriteManager::save(Module *c, const string &name) {
	if (!loaded)
		load(c->session);
	Favorite f;
	f.type = c->module_type;
	f.config_name = c->module_subtype;
	f.name = name;
	f.read_only = false;
	f.options = c->config_to_string();
	set(f);
	save(c->session);
}

void FavoriteManager::set(const Favorite &ff) {
	for (Favorite &f: favorites) {
		if ((f.type == ff.type) and (f.config_name == ff.config_name) and (f.name == ff.name)) {
			f.options = ff.options;
			return;
		}
	}

	favorites.add(ff);
}


class FavoriteSelectionDialog : public hui::Dialog {
public:
	FavoriteSelectionDialog(hui::Window *parent, const Array<string> &_names, bool _save) :
		hui::Dialog("favorite-dialog", parent)
	{
		save = _save;
		names = _names;
		set_options("name", "placeholder=" + _("enter new name"));
		if (!save)
			add_string("list", _("-Default  Parameters-"));
		for (string &n: names)
			add_string("list", n);
		if (!save)
			names.insert(":def:", 0);
		hide_control("name", !save);
		event("list", [=]{ on_list(); });
		event_x("list", "hui:select", [=]{ on_list_select(); });
		event("name", [=]{ on_name(); });
		event("ok", [=]{ on_ok(); });
		event("cancel", [=]{ destroy(); });
	}
	void on_list() {
		int n = get_int("list");
		selection = "";
		if (n >= 0) {
			selection = names[n];
			set_string("name", names[n]);
		}
		destroy();
	}
	void on_list_select() {
		int n = get_int("list");
		if (n >= 0)
			set_string("name", names[n]);
	}
	void on_name() {
		set_int("list", -1);
	}
	void on_ok() {
		selection = get_string("name");
		destroy();
	}

	bool save;
	Array<string> names;
	string selection;
};

string FavoriteManager::select_name(hui::Window *win, Module *c, bool save) {
	auto dlg = new FavoriteSelectionDialog(win, get_list(c), save);
	dlg->run();
	string sel = dlg->selection;
	delete dlg;
	return sel;
}
