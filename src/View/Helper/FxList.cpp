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


FxList::FxList(HuiWindow *_dlg, const string & _id, const string &_id_add, const string &_id_edit, const string &_id_delete)
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
	dlg->EventM(id, this, &FxList::OnList);
	dlg->EventMX(id, "hui:select", this, &FxList::OnListSelect);
	dlg->EventM(id_add, this, &FxList::OnAdd);
	dlg->EventM(id_edit, this, &FxList::OnEdit);
	dlg->EventM(id_delete, this, &FxList::OnDelete);
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
	msg_db_f("FillEffectList", 1);
	dlg->Reset(id);
	if (fx){
		foreach(Effect *f, *fx)
			dlg->AddString(id, f->name);
	}
	dlg->Enable(id, fx);
	dlg->Enable(id_add, fx);
	dlg->Enable(id_edit, false);
	dlg->Enable(id_delete, false);
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

bool FxList::UpdateEffectParams(Effect *f)
{
	msg_db_f("UpdateEffectParams", 1);
	bool ok = false;

	if (f->usable){

		f->ResetConfig();

		f->ConfigFromString();

		if (f->DoConfigure(false)){
			f->ConfigToString();
			ok = true;
		}
	}else{
		tsunami->log->Error(f->GetError());
	}

	return ok;
}

void FxList::AddNewEffect(string &filename)
{
	if (!fx)
		return;
	msg_db_f("AddNewEffect", 1);

	string name = filename.basename(); // remove directory
	name = name.substr(0, name.num - 5); //      and remove ".kaba"
	Effect *effect = CreateEffect(name);
	if (UpdateEffectParams(effect)){
		audio->Execute(new ActionTrackAddEffect(track, effect));
		FillList();
	}
}

void FxList::ExecuteFXDialog(int index)
{
	if (!fx)
		return;
	msg_db_f("ExecuteFXDialog", 1);

	Effect *f = (*fx)[index];
	f->ConfigToString();
	Array<EffectParam> param = f->param;
	if (UpdateEffectParams(f))
		audio->Execute(new ActionTrackEditEffect(track, index, param));
}
