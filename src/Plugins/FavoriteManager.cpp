/*
 * FavoriteManager.cpp
 *
 *  Created on: 13.04.2014
 *      Author: michi
 */

#include "FavoriteManager.h"
#include "../Module/Module.h"
#include "../lib/base/pointer.h"
#include "../lib/hui/hui.h"
#include "../lib/xfile/xml.h"
#include "../Tsunami.h"
#include "../Session.h"

const string FavoriteManager::DEFAULT_NAME = ":def:";

FavoriteManager::FavoriteManager() {
	loaded = false;
}

FavoriteManager::~FavoriteManager() {
}

void FavoriteManager::load_from_file(const Path &filename, bool read_only, Session *session) {
	if (!file_exists(filename))
		return;
	try {
		xml::Parser p;
		p.load(filename);
		if (p.elements.num == 0)
			throw Exception("no root element");
		auto &root = p.elements[0];
		auto *mm = root.find("list");
		for (auto &e: mm->elements) {
			Favorite ff;
			string cat = e.value("category");
			ff.type = Module::type_from_name(cat);
			ff.config_name = e.value("class");
			ff.name = e.value("name");
			ff.version = e.value("version")._int();
			ff.options = e.text;
			ff.read_only = read_only;
			set(ff);
		}
	} catch (Exception &e) {
		session->e("loading profile: " + e.message());
	}
}

void FavoriteManager::load(Session *session) {
	load_from_file(tsunami->directory_static << "profiles-demo.xml", true, session);
	load_from_file(tsunami->directory << "profiles.xml", false, session);
	save(session);
	loaded = true;
}

void FavoriteManager::save(Session *session) {
	try {
		xml::Parser p;
		xml::Element root("profiles");
		xml::Element hh("head");
		hh.add_attribute("version", "1.0");
		root.add(hh);
		xml::Element mm("list");
		for (Favorite &ff: favorites)
			if (!ff.read_only) {
				xml::Element e("profile");
				e.add_attribute("category", Module::type_to_name(ff.type));
				e.add_attribute("class", ff.config_name);
				e.add_attribute("version", i2s(ff.version));
				e.add_attribute("name", ff.name);
				e.text = ff.options;
				mm.add(e);
			}
		root.add(mm);
		p.elements.add(root);
		p.save(tsunami->directory << "profiles.xml");
	} catch (Exception &e) {
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

void FavoriteManager::apply(Module *c, const string &name, bool notify) {
	c->reset_config();
	if (name == DEFAULT_NAME) {
		if (notify)
			c->notify();
		return;
	}
	if (!loaded)
		load(c->session);
	for (Favorite &f: favorites) {
		if ((f.type == c->module_type) and (f.config_name == c->module_subtype) and (f.name == name))
			c->config_from_string(f.version, f.options);
	}
	if (notify)
		c->notify();
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
		event("cancel", [=]{ request_destroy(); });
	}
	void on_list() {
		int n = get_int("list");
		selection = "";
		if (n >= 0) {
			selection = names[n];
			set_string("name", names[n]);
		}
		request_destroy();
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
		request_destroy();
	}

	bool save;
	Array<string> names;
	string selection;
};

string FavoriteManager::select_name(hui::Window *win, Module *c, bool save) {
	auto dlg = ownify(new FavoriteSelectionDialog(win, get_list(c), save));
	dlg->run();
	return dlg->selection;
}
