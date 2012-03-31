/*
 * FxList.cpp
 *
 *  Created on: 31.03.2012
 *      Author: michi
 */

#include "FxList.h"
#include "../../Tsunami.h"



bool LoadAndCompileEffect(const string &filename)
{
	string _filename_ = HuiAppDirectoryStatic + "Plugins/All - " + filename + ".kaba";

	return tsunami->plugins->LoadAndCompilePlugin(_filename_);
}

FxList::FxList(CHuiWindow *_dlg, const string & _id, Array<Effect> & _fx):
fx(_fx)
{
	dlg = _dlg;
	id = _id;

	FillList();
	dlg->EventM(id, this, (void(HuiEventHandler::*)())&FxList::OnList);
}



void FxList::FillList()
{
	msg_db_r("FillEffectList", 1);
	dlg->Reset(id);
	foreach(fx, f)
		dlg->AddString(id, f.filename);
	dlg->AddString(id, _("  - neuer Effekt -"));
	msg_db_l(1);
}



void FxList::OnList()
{
	int s = dlg->GetInt(id);
	if (s >= fx.num){
		if (HuiFileDialogOpen(dlg, _("einen Effekt w&ahlen"), HuiAppDirectoryStatic + "Plugins", "*.kaba", "*.kaba"))
			AddNewEffect(HuiFilename);
	}else{
		ExecuteFXDialog(s);
		FillList();
	}

	/*float value = s->value_min + win->GetFloat("") * (s->value_max - s->value_min);
	win->SetFloat(s->id_edit, value * s->factor);
	if (s->func)
		s->func();*/
}

FxList::~FxList()
{
}

bool FxList::UpdateEffectParams(Effect &f, bool deletable)
{
	msg_db_r("UpdateEffectParams", 1);
	bool ok = false;


	if (LoadAndCompileEffect(f.filename)){

		tsunami->plugins->PluginResetData();

		tsunami->plugins->ImportPluginData(f);

		if (tsunami->plugins->PluginConfigure(deletable, false)){
			//cur_audio->history->Change();
			tsunami->plugins->ExportPluginData(f);
			ok = true;
		}
	}else{
		tsunami->log->Error(format(_("Fehler in  Script-Datei: \"%s\"\n%s\n%s"), f.filename.c_str(), tsunami->plugins->cur_plugin->s->ErrorMsgExt[0].c_str(), tsunami->plugins->cur_plugin->s->ErrorMsgExt[1].c_str()));
		if (deletable)
			tsunami->plugins->PluginDeleted = (HuiQuestionBox(HuiCurWindow, _("Frage"), _("Wollen Sie diesen Effekt l&oschen?")) == "hui:yes");
	}
	tsunami->plugins->PopCurPlugin();

	msg_db_l(1);
	return ok;
}

void FxList::AddNewEffect(string &filename)
{
	msg_db_r("AddNewEffect", 1);

	Effect effect;
	effect.filename = filename.substr(HuiAppDirectoryStatic.num + 8 + 6, -1); // remove directory    + "All - "
	effect.filename = effect.filename.substr(0, effect.filename.num - 5); //      and remove ".kaba"
	if (UpdateEffectParams(effect, false)){
		fx.add(effect);
		FillList();
	}

	msg_db_l(1);
}

void FxList::ExecuteFXDialog(int index)
{
	msg_db_r("ExecuteFXDialog", 1);

	UpdateEffectParams(fx[index], true);

	if (tsunami->plugins->PluginDeleted)
		fx.erase(index);

	msg_db_l(1);
}
