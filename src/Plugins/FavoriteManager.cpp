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
	CFile *f = FileOpen(filename);
	if (!f)
		return;
	int n = f->ReadInt();
	for (int i=0; i<n; i++){
		Favorite ff;
		string type = f->ReadStr();
		ff.type = str2type(type);
		ff.config_name = f->ReadStr();
		ff.name = f->ReadStr();
		ff.options = f->ReadStr();
		ff.read_only = read_only;
		set(ff);
	}
	delete(f);
}

void FavoriteManager::Load()
{
	LoadFromFile(HuiAppDirectoryStatic + "Data/favorites_demo.txt", true);
	LoadFromFile(HuiAppDirectory + "Data/favorites.txt", false);
	loaded = true;
}

void FavoriteManager::Save()
{
	CFile *f = FileCreate(HuiAppDirectory + "Data/favorites.txt");
	if (!f)
		return;
	f->WriteInt(favorites.num);
	foreach(Favorite &ff, favorites){
		f->WriteStr(type2str(ff.type));
		f->WriteStr(ff.config_name);
		f->WriteStr(ff.name);
		f->WriteStr(ff.options);
	}
	delete(f);
}

Array<string> FavoriteManager::GetList(Configurable *c)
{
	if (!loaded)
		Load();
	Array<string> r;
	foreach(Favorite &f, favorites){
		if ((f.type == c->configurable_type) && (f.config_name == c->name))
			r.add(f.name);
	}
	return r;
}

void FavoriteManager::Apply(Configurable *c, const string &name)
{
	c->ResetConfig();
	if (name == ":def:")
		return;
	msg_db_f("ApplyFavorite", 1);
	if (!loaded)
		Load();
	foreach(Favorite &f, favorites){
		if ((f.type == c->configurable_type) && (f.config_name == c->name) && (f.name == name))
			c->ConfigFromString(f.options);
	}
}

void FavoriteManager::Save(Configurable *c, const string &name)
{
	msg_db_f("SaveFavorite", 1);
	if (!loaded)
		Load();
	Favorite f;
	f.type = c->configurable_type;
	f.config_name = c->name;
	f.name = name;
	f.read_only = false;
	f.options = c->ConfigToString();
	set(f);
	Save();
}

void FavoriteManager::set(const Favorite &ff)
{
	foreach(Favorite &f, favorites){
		if ((f.type == ff.type) && (f.config_name == ff.config_name) && (f.name == ff.name)){
			f.options = ff.options;
			return;
		}
	}

	favorites.add(ff);
}


static string FavoriteSelectionDialogReturn;

class FavoriteSelectionDialog : public HuiDialog
{
public:
	FavoriteSelectionDialog(HuiWindow *win, const Array<string> &_names, bool _save) :
		HuiDialog(_(""), 300, 200, win, false)
	{
		save = _save;
		FavoriteSelectionDialogReturn = "";
		names = _names;
		AddControlTable("", 0, 0, 1, 2, "grid");
		SetTarget("grid", 0);
		AddListView("Name", 0, 0, 0, 0, "list");
		AddControlTable("", 0, 1, 2, 1, "grid2");
		SetTarget("grid2", 0);
		AddEdit("!expandx,placeholder=" + _("neuen Namen eingeben"), 0, 0, 0, 0, "name");
		AddDefButton("Ok", 1, 0, 0, 0, "ok");
		if (!save)
			AddString("list", _("-Standard Parameter-"));
		foreach(string &n, names)
			AddString("list", n);
		if (!save)
			names.insert(":def:", 0);
		HideControl("grid2", !save);
		EventM("list", this, &FavoriteSelectionDialog::onList);
		EventMX("list", "hui:select", this, &FavoriteSelectionDialog::onListSelect);
		EventM("name", this, &FavoriteSelectionDialog::onName);
		EventM("ok", this, &FavoriteSelectionDialog::onOk);
		SetInt("list", -1);
	}
	void onList()
	{
		int n = GetInt("list");
		FavoriteSelectionDialogReturn = "";
		if (n >= 0){
			FavoriteSelectionDialogReturn = names[n];
			SetString("name", names[n]);
		}
		delete(this);
	}
	void onListSelect()
	{
		int n = GetInt("list");
		if (n >= 0)
			SetString("name", names[n]);
	}
	void onName()
	{
		SetInt("list", -1);
	}
	void onOk()
	{
		FavoriteSelectionDialogReturn = GetString("name");
		delete(this);
	}

	bool save;
	Array<string> names;
};

string FavoriteManager::SelectName(HuiWindow *win, Configurable *c, bool save)
{
	FavoriteSelectionDialog *dlg = new FavoriteSelectionDialog(win, GetList(c), save);
	dlg->Run();
	return FavoriteSelectionDialogReturn;
}
