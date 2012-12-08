/*
 * FxList.cpp
 *
 *  Created on: 31.03.2012
 *      Author: michi
 */

#include "FxList.h"
#include "../../Tsunami.h"


FxList::FxList(CHuiWindow *_dlg, const string & _id, const string &_id_add, const string &_id_edit, const string &_id_delete, Array<Effect> & _fx):
fx(_fx)
{
	dlg = _dlg;
	id = _id;
	id_add = _id_add;
	id_edit = _id_edit;
	id_delete = _id_delete;

	FillList();
	dlg->EventM(id, this, (void(HuiEventHandler::*)())&FxList::OnList);
	dlg->EventMX(id, "hui:select", this, (void(HuiEventHandler::*)())&FxList::OnListSelect);
	dlg->EventM(id_add, this, (void(HuiEventHandler::*)())&FxList::OnAdd);
	dlg->EventM(id_edit, this, (void(HuiEventHandler::*)())&FxList::OnEdit);
	dlg->EventM(id_delete, this, (void(HuiEventHandler::*)())&FxList::OnDelete);
}



void FxList::FillList()
{
	msg_db_r("FillEffectList", 1);
	dlg->Reset(id);
	foreach(Effect &f, fx)
		dlg->AddString(id, f.name);
	dlg->Enable(id_edit, false);
	dlg->Enable(id_delete, false);
	msg_db_l(1);
}



void FxList::OnList()
{
	int s = dlg->GetInt(id);
	if (s >= 0){
		ExecuteFXDialog(s);
		FillList();
	}
}


void FxList::OnListSelect()
{
	int s = dlg->GetInt(id);
	dlg->Enable(id_edit, s >= 0);
	dlg->Enable(id_delete, s >= 0);
}


void FxList::OnAdd()
{
	if (HuiFileDialogOpen(dlg, _("einen Effekt w&ahlen"), HuiAppDirectoryStatic + "Plugins/Buffer/", "*.kaba", "*.kaba"))
		AddNewEffect(HuiFilename);
}


void FxList::OnEdit()
{
	int s = dlg->GetInt(id);
	if (s >= 0){
		ExecuteFXDialog(s);
		FillList();
	}
}


void FxList::OnDelete()
{
	int s = dlg->GetInt(id);
	if (s >= 0){
		fx.erase(s);
		FillList();
	}
}

FxList::~FxList()
{
}

bool FxList::UpdateEffectParams(Effect &f)
{
	msg_db_r("UpdateEffectParams", 1);
	bool ok = false;


	if (tsunami->plugins->LoadAndCompileEffect(f)){

		f.plugin->ResetData();

		tsunami->plugins->ImportPluginData(f);

		if (f.plugin->Configure(false)){
			tsunami->plugins->ExportPluginData(f);
			ok = true;
		}
	}else{
		tsunami->log->Error(format(_("Fehler in  Script-Datei: \"%s\"\n%s\n%s"), f.name.c_str(), tsunami->plugins->cur_plugin->s->ErrorMsgExt[0].c_str(), tsunami->plugins->cur_plugin->s->ErrorMsgExt[1].c_str()));
	}

	msg_db_l(1);
	return ok;
}

void FxList::AddNewEffect(string &filename)
{
	msg_db_r("AddNewEffect", 1);

	Effect effect;
	effect.name = filename.basename(); // remove directory
	effect.name = effect.name.substr(0, effect.name.num - 5); //      and remove ".kaba"
	if (UpdateEffectParams(effect)){
		fx.add(effect);
		FillList();
	}

	msg_db_l(1);
}

void FxList::ExecuteFXDialog(int index)
{
	msg_db_r("ExecuteFXDialog", 1);

	UpdateEffectParams(fx[index]);

	msg_db_l(1);
}
