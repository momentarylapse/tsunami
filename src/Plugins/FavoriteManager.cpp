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
}

FavoriteManager::~FavoriteManager()
{
}


string get_fav_dir(Configurable *c)
{
	dir_create(HuiAppDirectory + "Favorites");
	if (c->configurable_type == CONFIGURABLE_SYNTHESIZER){
		dir_create(HuiAppDirectory + "Favorites/Synthesizer");
		return HuiAppDirectory + "Favorites/Synthesizer";
	}
	dir_create(HuiAppDirectory + "Favorites/Effect");
	return HuiAppDirectory + "Favorites/Effect";
}

Array<string> FavoriteManager::GetList(Configurable *c)
{
	Array<string> names;

	string init = c->name + "___";

	string dir = get_fav_dir(c);
	Array<DirEntry> list = dir_search(dir, "*", false);
	foreach(DirEntry &e, list){
		if (e.name.find(init) < 0)
			continue;
		names.add(e.name.substr(init.num, -1));
	}
	return names;
}

void FavoriteManager::Apply(Configurable *c, const string &name)
{
	c->ResetConfig();
	if (name == ":def:")
		return;
	msg_db_f("ApplyFavorite", 1);
	string dir = get_fav_dir(c);
	CFile *f = FileOpen(dir + "/" + c->name + "___" + name);
	if (!f)
		return;
	c->ConfigFromString(f->ReadStr());
	delete(f);
}

void FavoriteManager::Save(Configurable *c, const string &name)
{
	msg_db_f("SaveFavorite", 1);
	string dir = get_fav_dir(c);
	CFile *f = FileCreate(dir + "/" + c->name + "___" + name);
	f->WriteStr(c->ConfigToString());
	delete(f);
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
		EventM("list", this, &FavoriteSelectionDialog::OnList);
		EventMX("list", "hui:select", this, &FavoriteSelectionDialog::OnListSelect);
		EventM("name", this, &FavoriteSelectionDialog::OnName);
		EventM("ok", this, &FavoriteSelectionDialog::OnOk);
		SetInt("list", -1);
	}
	void OnList()
	{
		int n = GetInt("list");
		FavoriteSelectionDialogReturn = "";
		if (n >= 0){
			FavoriteSelectionDialogReturn = names[n];
			SetString("name", names[n]);
		}
		delete(this);
	}
	void OnListSelect()
	{
		int n = GetInt("list");
		if (n >= 0)
			SetString("name", names[n]);
	}
	void OnName()
	{
		SetInt("list", -1);
	}
	void OnOk()
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
