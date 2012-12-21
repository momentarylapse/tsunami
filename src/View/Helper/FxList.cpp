/*
 * FxList.cpp
 *
 *  Created on: 31.03.2012
 *      Author: michi
 */

#include "FxList.h"
#include "../../Data/AudioFile.h"
#include "../../Plugins/Plugin.h"
#include "../../Plugins/Effect.h"
#include "../../Tsunami.h"
#include "../../Stuff/Log.h"
#include "../../Action/Track/Effect/ActionTrackAddEffect.h"
#include "../../Action/Track/Effect/ActionTrackDeleteEffect.h"
#include "../../Action/Track/Effect/ActionTrackEditEffect.h"


FxList::FxList(CHuiWindow *_dlg, const string & _id, const string &_id_add, const string &_id_edit, const string &_id_delete)
{
	dlg = _dlg;
	id = _id;
	id_add = _id_add;
	id_edit = _id_edit;
	id_delete = _id_delete;

	audio = NULL;
	track = NULL;
	fx = NULL;

	FillList();
	dlg->EventM(id, this, (void(HuiEventHandler::*)())&FxList::OnList);
	dlg->EventMX(id, "hui:select", this, (void(HuiEventHandler::*)())&FxList::OnListSelect);
	dlg->EventM(id_add, this, (void(HuiEventHandler::*)())&FxList::OnAdd);
	dlg->EventM(id_edit, this, (void(HuiEventHandler::*)())&FxList::OnEdit);
	dlg->EventM(id_delete, this, (void(HuiEventHandler::*)())&FxList::OnDelete);
}


void FxList::SetAudio(AudioFile *a)
{
	audio = a;
	track = NULL;
	fx = NULL;
	if (a)
		fx = &a->fx;
	FillList();
}

void FxList::SetTrack(Track *t)
{
	audio = NULL;
	track = t;
	fx = NULL;
	if (t){
		audio = t->root;
		fx = &t->fx;
	}
	FillList();
}

void FxList::FillList()
{
	msg_db_r("FillEffectList", 1);
	dlg->Reset(id);
	if (fx){
		foreach(Effect &f, *fx)
			dlg->AddString(id, f.name);
	}
	dlg->Enable(id, fx);
	dlg->Enable(id_add, fx);
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
	if (!fx)
		return;
	if (HuiFileDialogOpen(dlg, _("einen Effekt w&ahlen"), HuiAppDirectoryStatic + "Plugins/Buffer/", "*.kaba", "*.kaba"))
		AddNewEffect(HuiFilename);
}


void FxList::OnEdit()
{
	if (!fx)
		return;
	int s = dlg->GetInt(id);
	if (s >= 0){
		ExecuteFXDialog(s);
		FillList();
	}
}


void FxList::OnDelete()
{
	if (!fx)
		return;
	int s = dlg->GetInt(id);
	if (s >= 0){
		audio->Execute(new ActionTrackDeleteEffect(track, s));
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

	f.make_usable();

	if (f.usable){

		f.plugin->ResetData();

		f.ImportData();

		if (f.plugin->Configure(false)){
			f.ExportData();
			ok = true;
		}
	}else{
		tsunami->log->Error(f.GetError());
	}

	msg_db_l(1);
	return ok;
}

void FxList::AddNewEffect(string &filename)
{
	if (!fx)
		return;
	msg_db_r("AddNewEffect", 1);

	Effect effect;
	effect.name = filename.basename(); // remove directory
	effect.name = effect.name.substr(0, effect.name.num - 5); //      and remove ".kaba"
	if (UpdateEffectParams(effect)){
		audio->Execute(new ActionTrackAddEffect(track, effect));
		FillList();
	}

	msg_db_l(1);
}

void FxList::ExecuteFXDialog(int index)
{
	if (!fx)
		return;
	msg_db_r("ExecuteFXDialog", 1);

	Effect temp = (*fx)[index];
	UpdateEffectParams(temp);
	audio->Execute(new ActionTrackEditEffect(track, index, temp));

	msg_db_l(1);
}
