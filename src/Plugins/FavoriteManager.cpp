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

FavoriteManager::FavoriteManager()
{
	loaded = false;
}

FavoriteManager::~FavoriteManager()
{
}

void FavoriteManager::LoadFromFile(const string &filename, bool read_only, Session *session)
{
	if (!file_test_existence(filename))
		return;
	try{
		File *f = FileOpenText(filename);
		int n = f->read_int();
		for (int i=0; i<n; i++){
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
	}catch(Exception &e){
		session->e(e.message());
	}
}

void FavoriteManager::Load(Session *session)
{
	LoadFromFile(tsunami->directory_static + "favorites_demo.txt", true, session);
	LoadFromFile(tsunami->directory + "favorites.txt", false, session);
	loaded = true;
}

void FavoriteManager::Save(Session *session)
{
	try{
		File *f = FileCreateText(tsunami->directory + "favorites.txt");
		f->write_int(favorites.num);
		for (Favorite &ff: favorites){
			f->write_str(Module::type_to_name(ff.type));
			f->write_str(ff.config_name);
			f->write_str(ff.name);
			f->write_str(ff.options);
		}
		delete(f);
	}catch(Exception &e){
		session->e(e.message());
	}
}

Array<string> FavoriteManager::GetList(Module *c)
{
	if (!loaded)
		Load(c->session);
	Array<string> r;
	for (Favorite &f: favorites){
		if ((f.type == c->module_type) and (f.config_name == c->module_subtype))
			r.add(f.name);
	}
	return r;
}

void FavoriteManager::Apply(Module *c, const string &name)
{
	c->reset_config();
	if (name == DEFAULT_NAME)
		return;
	if (!loaded)
		Load(c->session);
	for (Favorite &f: favorites){
		if ((f.type == c->module_type) and (f.config_name == c->module_subtype) and (f.name == name))
			c->config_from_string(f.options);
	}
}

void FavoriteManager::Save(Module *c, const string &name)
{
	if (!loaded)
		Load(c->session);
	Favorite f;
	f.type = c->module_type;
	f.config_name = c->module_subtype;
	f.name = name;
	f.read_only = false;
	f.options = c->config_to_string();
	set(f);
	Save(c->session);
}

void FavoriteManager::set(const Favorite &ff)
{
	for (Favorite &f: favorites){
		if ((f.type == ff.type) and (f.config_name == ff.config_name) and (f.name == ff.name)){
			f.options = ff.options;
			return;
		}
	}

	favorites.add(ff);
}


class FavoriteSelectionDialog : public hui::Dialog
{
public:
	FavoriteSelectionDialog(hui::Window *win, const Array<string> &_names, bool _save) :
		hui::Dialog(_(""), 300, 200, win, false)
	{
		save = _save;
		names = _names;
		addGrid("", 0, 0, "grid");
		setTarget("grid");
		addListView("Name", 0, 0, "list");
		addGrid("", 0, 1, "grid2");
		setTarget("grid2");
		addEdit("!expandx,placeholder=" + _("enter new name"), 0, 0, "name");
		addDefButton("Ok", 1, 0, "ok");
		if (!save)
			addString("list", _("-Default  Parameters-"));
		for (string &n: names)
			addString("list", n);
		if (!save)
			names.insert(":def:", 0);
		hideControl("grid2", !save);
		event("list", std::bind(&FavoriteSelectionDialog::onList, this));
		eventX("list", "hui:select", std::bind(&FavoriteSelectionDialog::onListSelect, this));
		event("name", std::bind(&FavoriteSelectionDialog::onName, this));
		event("ok", std::bind(&FavoriteSelectionDialog::onOk, this));
		setInt("list", -1);
	}
	void onList()
	{
		int n = getInt("list");
		selection = "";
		if (n >= 0){
			selection = names[n];
			setString("name", names[n]);
		}
		destroy();
	}
	void onListSelect()
	{
		int n = getInt("list");
		if (n >= 0)
			setString("name", names[n]);
	}
	void onName()
	{
		setInt("list", -1);
	}
	void onOk()
	{
		selection = getString("name");
		destroy();
	}

	bool save;
	Array<string> names;
	string selection;
};

string FavoriteManager::SelectName(hui::Window *win, Module *c, bool save)
{
	FavoriteSelectionDialog *dlg = new FavoriteSelectionDialog(win, GetList(c), save);
	dlg->run();
	string sel = dlg->selection;
	delete(dlg);
	return sel;
}
