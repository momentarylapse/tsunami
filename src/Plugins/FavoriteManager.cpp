/*
 * FavoriteManager.cpp
 *
 *  Created on: 13.04.2014
 *      Author: michi
 */

#include "FavoriteManager.h"
#include "Configurable.h"
#include "../lib/file/file.h"
#include "../lib/hui/hui.h"
#include "../Tsunami.h"
#include "../Stuff/Log.h"

FavoriteManager::FavoriteManager()
{
	loaded = false;
}

FavoriteManager::~FavoriteManager()
{
}

string FavoriteManager::type2str(int type)
{
	if (type == Configurable::TYPE_EFFECT)
		return "Effect";
	if (type == Configurable::TYPE_SYNTHESIZER)
		return "Synth";
	if (type == Configurable::TYPE_MIDI_EFFECT)
		return "MidiEffect";
	return "???";
}

int FavoriteManager::str2type(const string &str)
{
	if (str == "Effect")
		return Configurable::TYPE_EFFECT;
	if (str == "Synth")
		return Configurable::TYPE_SYNTHESIZER;
	if (str == "MidiEffect")
		return Configurable::TYPE_MIDI_EFFECT;
	return -1;
}

void FavoriteManager::LoadFromFile(const string &filename, bool read_only)
{
	if (!file_test_existence(filename))
		return;
	try{
		File *f = FileOpenText(filename);
		int n = f->read_int();
		for (int i=0; i<n; i++){
			Favorite ff;
			string type = f->read_str();
			ff.type = str2type(type);
			ff.config_name = f->read_str();
			ff.name = f->read_str();
			ff.options = f->read_str();
			ff.read_only = read_only;
			set(ff);
		}
		delete(f);
	}catch(Exception &e){
		tsunami->log->error(e.message());
	}
}

void FavoriteManager::Load()
{
	LoadFromFile(tsunami->directory_static + "favorites_demo.txt", true);
	LoadFromFile(tsunami->directory + "favorites.txt", false);
	loaded = true;
}

void FavoriteManager::Save()
{
	try{
		File *f = FileCreateText(tsunami->directory + "favorites.txt");
		f->write_int(favorites.num);
		for (Favorite &ff: favorites){
			f->write_str(type2str(ff.type));
			f->write_str(ff.config_name);
			f->write_str(ff.name);
			f->write_str(ff.options);
		}
		delete(f);
	}catch(Exception &e){
		tsunami->log->error(e.message());
	}
}

Array<string> FavoriteManager::GetList(Configurable *c)
{
	if (!loaded)
		Load();
	Array<string> r;
	for (Favorite &f: favorites){
		if ((f.type == c->configurable_type) and (f.config_name == c->name))
			r.add(f.name);
	}
	return r;
}

void FavoriteManager::Apply(Configurable *c, const string &name)
{
	c->resetConfig();
	if (name == ":def:")
		return;
	if (!loaded)
		Load();
	for (Favorite &f: favorites){
		if ((f.type == c->configurable_type) and (f.config_name == c->name) and (f.name == name))
			c->configFromString(f.options);
	}
}

void FavoriteManager::Save(Configurable *c, const string &name)
{
	if (!loaded)
		Load();
	Favorite f;
	f.type = c->configurable_type;
	f.config_name = c->name;
	f.name = name;
	f.read_only = false;
	f.options = c->configToString();
	set(f);
	Save();
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

string FavoriteManager::SelectName(hui::Window *win, Configurable *c, bool save)
{
	FavoriteSelectionDialog *dlg = new FavoriteSelectionDialog(win, GetList(c), save);
	dlg->run();
	string sel = dlg->selection;
	delete(dlg);
	return sel;
}
