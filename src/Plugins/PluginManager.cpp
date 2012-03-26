/*
 * PluginManager.cpp
 *
 *  Created on: 26.03.2012
 *      Author: michi
 */

#include "PluginManager.h"
#include "../Tsunami.h"
#include "FastFourierTransform.h"
#include "../View/Dialog/Slider.h"

PluginManager::PluginManager()
{
}

PluginManager::~PluginManager()
{
}


void PluginManager::PushCurPlugin(CScript *s)
{
	foreachi(plugin, p, i)
		if (s == p.s){
			cur_plugin_stack.add(i);
			cur_plugin = &p;
		}
}

void PluginManager::PopCurPlugin()
{
	cur_plugin_stack.resize(cur_plugin_stack.num - 1);
	if (cur_plugin_stack.num > 0){
		int i = cur_plugin_stack.back();
		cur_plugin = &plugin[i];
	}else{
		cur_plugin = NULL;
		//DeleteAllScripts();
		//LoadedScript.clear();
		//msg_write("---DeleteAll--------");
	}
}

BufferBox TrackGetBuffers(Track *t, int pos, int length)
{	return t->GetBuffers(pos, length);	}

BufferBox TrackReadBuffers(Track *t, int pos, int length)
{	return t->ReadBuffers(pos, length);	}

BufferBox AudioFileRender(AudioFile *a, int pos, int length)
{	return tsunami->renderer->RenderAudioFile(a, pos, length);	}

void GlobalPutFavoriteBarFixed(CHuiWindow *win, int x, int y, int w)
{	tsunami->plugins->PutFavoriteBarFixed(win, x, y, w);	}

void GlobalPutFavoriteBarSizable(CHuiWindow *win, const string &root_id, int x, int y)
{	tsunami->plugins->PutFavoriteBarSizable(win, root_id, x, y);	}

void GlobalPutCommandBarFixed(CHuiWindow *win, int x, int y, int w)
{	tsunami->plugins->PutCommandBarFixed(win, x, y, w);	}

void GlobalPutCommandBarSizable(CHuiWindow *win, const string &root_id, int x, int y)
{	tsunami->plugins->PutCommandBarSizable(win, root_id, x, y);	}

Array<Slider*> global_slider;

void GlobalAddSlider(CHuiWindow *win, const string &id_slider, const string &id_edit, float v_min, float v_max, float factor, hui_callback *func, float value)
{	global_slider.add(new Slider(win, id_slider, id_edit, v_min, v_max, factor, func, value));	}

void GlobalSliderSet(CHuiWindow *win, const string &id, float value)
{
	foreach(global_slider, s)
		if (s->Match(id))
				s->Set(value);
}

float GlobalSliderGet(CHuiWindow *win, const string &id)
{
	foreach(global_slider, s)
		if (s->Match(id))
				return s->Get();
	return 0;
}

void GlobalRemoveSliders(CHuiWindow *win)
{
	foreach(global_slider, g)
		delete(g);
	global_slider.clear();
}

CHuiWindow *GlobalMainWin = NULL;

void PluginManager::LinkAppScriptData()
{
	msg_db_r("LinkAppScriptData", 2);
	ScriptDirectory = "";

	// api definition
	ScriptResetSemiExternalData();
	GlobalMainWin = dynamic_cast<CHuiWindow*>(tsunami);
	ScriptLinkSemiExternalVar("MainWin",		&GlobalMainWin);
	ScriptLinkSemiExternalVar("cur_audio",		&tsunami->cur_audio);
/*	ScriptLinkSemiExternalVar("CaptureBuf",		&CaptureBuf);
	ScriptLinkSemiExternalVar("CaptureAddData",	&CaptureAddData);
	ScriptLinkSemiExternalVar("CapturePreviewBuf",&CapturePreviewBuf);
	ScriptLinkSemiExternalFunc("CreateNewAudioFile",(void*)&CreateNewAudioFile);
	ScriptLinkSemiExternalFunc("AddEmptyTrack",	(void*)&AddEmptyTrack);
	ScriptLinkSemiExternalFunc("DeleteTrack",	(void*)&DeleteTrack);
	ScriptLinkSemiExternalFunc("AddEmptySubTrack",(void*)&AddEmptySubTrack);*/
	ScriptLinkSemiExternalFunc("TrackGetBuffers",	(void*)&TrackGetBuffers);
	ScriptLinkSemiExternalFunc("TrackReadBuffers",	(void*)&TrackReadBuffers);
//	ScriptLinkSemiExternalFunc("UpdatePeaks",	(void*)&UpdatePeaks);
//	ScriptLinkSemiExternalFunc("ChangeAudioFile",(void*)&ChangeAudioFile);
//	ScriptLinkSemiExternalFunc("ChangeTrack",	(void*)&ChangeTrack);
	ScriptLinkSemiExternalFunc("fft_c2c",		(void*)&FastFourierTransform::fft_c2c);
	ScriptLinkSemiExternalFunc("fft_r2c",		(void*)&FastFourierTransform::fft_r2c);
	ScriptLinkSemiExternalFunc("fft_c2r_inv",	(void*)&FastFourierTransform::fft_c2r_inv);
	ScriptLinkSemiExternalFunc("fft_i2c",		(void*)&FastFourierTransform::fft_i2c);
	ScriptLinkSemiExternalFunc("fft_c2i_inv",	(void*)&FastFourierTransform::fft_c2i_inv);
/*	ScriptLinkSemiExternalFunc("CaptureStart",	(void*)&CaptureStart);
	ScriptLinkSemiExternalFunc("CaptureStop",	(void*)&CaptureStop);
	ScriptLinkSemiExternalFunc("DoCapturing",	(void*)&DoCapturing);
	ScriptLinkSemiExternalFunc("ProgressStart",	(void*)&ProgressStart);
	ScriptLinkSemiExternalFunc("ProgressEnd",	(void*)&ProgressEnd);
	ScriptLinkSemiExternalFunc("Progress",		(void*)&ProgressStatus);*/
	ScriptLinkSemiExternalFunc("PutFavoriteBarFixed",	(void*)&GlobalPutFavoriteBarFixed);
	ScriptLinkSemiExternalFunc("PutFavoriteBarSizable",	(void*)&GlobalPutFavoriteBarSizable);
	ScriptLinkSemiExternalFunc("PutCommandBarFixed",	(void*)&GlobalPutCommandBarFixed);
	ScriptLinkSemiExternalFunc("PutCommandBarSizable",	(void*)&GlobalPutCommandBarSizable);
	ScriptLinkSemiExternalFunc("AddSlider",		(void*)&GlobalAddSlider);
	ScriptLinkSemiExternalFunc("SliderSet",		(void*)&GlobalSliderSet);
	ScriptLinkSemiExternalFunc("SliderGet",		(void*)&GlobalSliderGet);
	ScriptLinkSemiExternalFunc("RemoveSliders",	(void*)&GlobalRemoveSliders);
	ScriptLinkSemiExternalFunc("AudioFileRender",		(void*)&AudioFileRender);
	msg_db_l(2);
}

void PluginManager::OnMenuExecutePlugin()
{
	int n = s2i(HuiGetEvent()->id.substr(strlen("execute_plugin_"), -1));

	if ((n >= 0) && (n <PluginFile.num))
		ExecutePlugin(HuiAppDirectoryStatic + "Plugins/" + PluginFile[n]);
}

void PluginManager::AddPluginsToMenu()
{
	msg_db_r("AddPluginsToMenu", 2);
	ScriptInit();

	// alle finden
	int n = dir_search(HuiAppDirectoryStatic + "Plugins", "*.kaba", false);
	CHuiMenu *m = tsunami->GetMenu()->GetSubMenuByID("menu_plugins");
	for (int i=0;i<n;i++)
		if (dir_search_name[i] != "api.kaba"){
			PluginFile.add(dir_search_name[i]);
		}

	// "All - "..
	n = 0;
	for (int i=0;i<PluginFile.num;i++)
		if (PluginFile[i].find("All - ") == 0){
			if (n == 0){
				m->AddSeparator();
				m->AddItem(_("Auf Audiopuffer"), "plugin_on_file");
				m->EnableItem("plugin_on_file", false);
			}
			m->AddItem(PluginFile[i].substr(6, PluginFile[i].num - 11), format("execute_plugin_%d", i));
			n ++;
		}

	// "Track - "..
	n = 0;
	for (int i=0;i<PluginFile.num;i++)
		if (PluginFile[i].find("Track - ") == 0){
			if (n == 0){
				m->AddSeparator();
				m->AddItem(_("Auf einzelne Spur"), "plugin_on_track");
				m->EnableItem("plugin_on_track", false);
			}
			m->AddItem(PluginFile[i].substr(8, PluginFile[i].num - 13), format("execute_plugin_%d", i));
			n ++;
		}

	// rest
	n = 0;
	for (int i=0;i<PluginFile.num;i++)
		if ((PluginFile[i].find("All - ") != 0) && (PluginFile[i].find("Track - ") != 0) && (PluginFile[i].find("Effect - ") != 0)){
			if (n == 0){
				m->AddSeparator();
				m->AddItem(_("Sonstige"), "plugin_other");
				m->EnableItem("plugin_other", false);
			}
			m->AddItem(PluginFile[i].substr(0, PluginFile[i].num - 5), format("execute_plugin_%d", i));
			n ++;
		}

	// Events
	for (int i=0;i<PluginFile.num;i++)
		tsunami->EventM(format("execute_plugin_%d", i), this, (void(HuiEventHandler::*)())&PluginManager::OnMenuExecutePlugin);
	msg_db_l(2);
}

void PluginManager::InitPluginData()
{
	msg_db_r("InitPluginData", 2);
	msg_db_l(2);
}

void PluginManager::FinishPluginData()
{
	msg_db_r("FinishPluginData", 2);
	tsunami->ForceRedraw();
	msg_db_l(2);
}

void PluginManager::OnFavoriteName()
{
	bool enabled = HuiCurWindow->GetString("").num > 0;
	HuiCurWindow->Enable("favorite_save", enabled);
	HuiCurWindow->Enable("favorite_delete", enabled);
}

void PluginManager::OnFavoriteList()
{
	int n = HuiCurWindow->GetInt("");
	PluginResetData();
	if (n == 0){
		HuiCurWindow->SetString("favorite_name", "");
		HuiCurWindow->Enable("favorite_save", false);
		HuiCurWindow->Enable("favorite_delete", false);
	}else{
		LoadPluginDataFromFile(PluginFavoriteName[n - 1]);
		HuiCurWindow->SetString("favorite_name", PluginFavoriteName[n - 1]);
		HuiCurWindow->Enable("favorite_delete", true);
	}
	PluginDataToDialog();
}

void PluginManager::OnFavoriteSave()
{
	string name = HuiCurWindow->GetString("favorite_name");
	WritePluginDataToFile(name);
	PluginFavoriteName.add(name);
	HuiCurWindow->AddString("favorite_list", name);
	HuiCurWindow->SetInt("favorite_list", PluginFavoriteName.num);
}

void PluginManager::OnFavoriteDelete()
{}

void PluginManager::InitFavorites(CHuiWindow *win)
{
	msg_db_r("InitFavorites", 1);
	PluginFavoriteName.clear();


	win->Enable("favorite_save", false);
	win->Enable("favorite_delete", false);

	string init = basename(cur_plugin->s->pre_script->Filename) + "___";

	dir_create(HuiAppDirectory + "Plugins/Favorites");
	int n = dir_search(HuiAppDirectory + "Plugins/Favorites", "*", false);
	for (int i=0;i<n;i++){
		if (dir_search_name[i].find(init) < 0)
			continue;
		PluginFavoriteName.add(dir_search_name[i].substr(init.num, -1));
		win->AddString("favorite_list", PluginFavoriteName.back());
	}

	win->EventM("favorite_name", this, (void(HuiEventHandler::*)())&PluginManager::OnFavoriteName);
	win->EventM("favorite_save", this, (void(HuiEventHandler::*)())&PluginManager::OnFavoriteSave);
	win->EventM("favorite_delete", this, (void(HuiEventHandler::*)())&PluginManager::OnFavoriteDelete);
	win->EventM("favorite_list", this, (void(HuiEventHandler::*)())&PluginManager::OnFavoriteList);
	msg_db_l(1);
}

void PluginManager::PutFavoriteBarFixed(CHuiWindow *win, int x, int y, int w)
{
	msg_db_r("PutFavoriteBarFixed", 1);
	w -= 10;
	win->AddComboBox("", x, y, w / 2 - 35, 25, "favorite_list");
	win->AddEdit("", x + w / 2 - 30, y, w / 2 - 30, 25, "favorite_name");
	win->AddButton("", x + w - 55, y, 25, 25, "favorite_save");
	win->SetImage("favorite_save", "hui:save");
	win->AddButton("", x + w - 25, y, 25, 25, "favorite_delete");
	win->SetImage("favorite_delete", "hui:delete");

	InitFavorites(win);
	msg_db_l(1);
}

void PluginManager::PutFavoriteBarSizable(CHuiWindow *win, const string &root_id, int x, int y)
{
	msg_db_r("PutFavoriteBarSizable", 1);
	win->SetTarget(root_id, 0);
	win->AddControlTable("", x, y, 4, 1, "favorite_table");
	win->SetTarget("favorite_table", 0);
	win->AddComboBox("", 0, 0, 0, 0, "favorite_list");
	win->AddEdit("", 1, 0, 0, 0, "favorite_name");
	win->AddButton("", 2, 0, 0, 0, "favorite_save");
	win->SetImage("favorite_save", "hui:save");
	win->AddButton("", 3, 0, 0, 0, "favorite_delete");
	win->SetImage("favorite_delete", "hui:delete");

	InitFavorites(win);
	msg_db_l(1);
}

void PluginManager::OnPluginFavoriteName()
{
	CHuiWindow *win = HuiGetEvent()->win;
	win->Enable("favorite_save", win->GetString("favorite_name").num > 0);
	win->Enable("favorite_delete", win->GetString("favorite_name").num > 0);
}

void PluginManager::OnPluginFavoriteList()
{
	CHuiWindow *win = HuiGetEvent()->win;
	int n = win->GetInt("");
	PluginResetData();
	if (n == 0){
		win->SetString("favorite_name", "");
		win->Enable("favorite_save", false);
		win->Enable("favorite_delete", false);
	}else{
		LoadPluginDataFromFile(PluginFavoriteName[n - 1].c_str());
		win->SetString("favorite_name", PluginFavoriteName[n - 1].c_str());
		win->Enable("favorite_delete", true);
	}
	PluginDataToDialog();
}

void PluginManager::OnPluginFavoriteSave()
{
	CHuiWindow *win = HuiGetEvent()->win;
	WritePluginDataToFile(win->GetString("favorite_name"));
	PluginFavoriteName.add(win->GetString("favorite_name"));
	win->AddString("favorite_list", win->GetString("favorite_name"));
	win->SetInt("favorite_list", PluginFavoriteName.num);
}

void PluginManager::OnPluginOk()
{
	PluginCancelled = false;
	PluginDeleted = false;
	delete(HuiCurWindow);
}

void PluginManager::OnPluginDelete()
{
	PluginCancelled = true;
	PluginDeleted = true;
	delete(HuiCurWindow);
}

void PluginManager::OnPluginClose()
{
	PluginCancelled = true;
	PluginDeleted = false;
	delete(HuiCurWindow);
}

void PluginManager::PutCommandBarFixed(CHuiWindow *win, int x, int y, int w)
{
	msg_db_r("PutCommandBarFixed", 1);
	w -= 10;
	int ww = (w - 30) / 3;
	if (ww > 120)
		ww = 120;

	win->AddDefButton(_("OK"),w - ww,y,ww,25,"ok");
	//win->SetImage("ok", "hui:ok");
	win->AddButton(_("Abbrechen"),w - ww*2 - 10,y,ww,25,"cancel");
	//win->SetImage("cancel", "hui:cancel");

	if (PluginAddDelete){
		win->AddButton(_("L&oschen"),w - ww * 3 - 20,y,ww,25,"delete");
		win->SetImage("delete", "hui:delete");
	}else if (PluginAddPreview){
		if (cur_plugin->s->pre_script->Filename.find("All - ") >= 0){
			win->AddButton(_("Vorschau"),w - ww * 3 - 20,y,ww,25,"preview");
			win->SetImage("preview", "hui:media-play");
		}
	}
	win->EventM("ok", this, (void(HuiEventHandler::*)())&PluginManager::OnPluginOk);
	win->EventM("delete", this, (void(HuiEventHandler::*)())&PluginManager::OnPluginDelete);
	win->EventM("preview", this, (void(HuiEventHandler::*)())&PluginManager::PluginPreview);
	win->EventM("cancel", this, (void(HuiEventHandler::*)())&PluginManager::OnPluginClose);
	win->EventM("hui:close", this, (void(HuiEventHandler::*)())&PluginManager::OnPluginClose);
	msg_db_l(1);
}

void PluginManager::PutCommandBarSizable(CHuiWindow *win, const string &root_id, int x, int y)
{
	msg_db_r("PutCommandBarSizable", 1);
	win->SetTarget(root_id, 0);
	win->AddControlTable("", x, y, 4, 1, "command_table");
	win->SetTarget("command_table", 0);
	win->AddDefButton(_("OK"), 3, 0, 0, 0, "ok");
	win->SetImage("ok", "hui:ok");
	win->AddButton(_("Abbrechen"), 2, 0, 0, 0, "cancel");
	win->SetImage("cancel", "hui:cancel");
	win->AddText("", 1, 0, 0, 0, "");
	if (PluginAddDelete){
		win->AddButton(_("L&oschen"), 0, 0, 0, 0, "delete");
		win->SetImage("delete", "hui:delete");
	}else if (PluginAddPreview){
		if (cur_plugin->s->pre_script->Filename.find("All - ") >= 0){
			win->AddButton(_("Vorschau"), 0, 0, 0, 0, "preview");
			win->SetImage("preview", "hui:media-play");
		}
	}
	win->EventM("ok", this, (void(HuiEventHandler::*)())&PluginManager::OnPluginOk);
	win->EventM("delete", this, (void(HuiEventHandler::*)())&PluginManager::OnPluginDelete);
	win->EventM("preview", this, (void(HuiEventHandler::*)())&PluginManager::PluginPreview);
	win->EventM("cancel", this, (void(HuiEventHandler::*)())&PluginManager::OnPluginClose);
	win->EventM("hui:close", this, (void(HuiEventHandler::*)())&PluginManager::OnPluginClose);
	msg_db_l(1);
}

void PluginManager::PluginResetData()
{
	if (cur_plugin)
		if (cur_plugin->f_reset)
			cur_plugin->f_reset();
}

bool PluginManager::PluginConfigure(bool deletable, bool previewable)
{
	msg_db_r("PluginConfigure", 1);
	if (cur_plugin->f_configure){
		PluginAddDelete = deletable;
		PluginAddPreview = previewable;
		cur_plugin->f_configure();
		GlobalRemoveSliders(NULL);
		msg_db_l(1);
		return !PluginCancelled;
	}else if (deletable){
		PluginDeleted = (HuiQuestionBox(HuiCurWindow, _("Frage"), _("Dieser Effekt ist nicht konfigurierbar.\nWollen Sie ihn l&oschen?")) == "yes");
	}
	msg_db_l(1);
	return true;
}

void PluginManager::PluginDataToDialog()
{
	if (cur_plugin->f_data2dialog)
		cur_plugin->f_data2dialog();
}

void try_write_primitive_element(string &var_temp, sType *t, char *v)
{
	if (t == TypeInt)
		var_temp += i2s(*(int*)v);
	else if (t == TypeChar)
		var_temp += i2s(*(char*)v);
	else if (t == TypeFloat)
		var_temp += f2s(*(float*)v, 6);
	else if (t == TypeBool)
		var_temp += (*(bool*)v) ? "true" : "false";
	else if (t == TypeVector)
		var_temp += format("(%f %f %f)", *(float*)v, ((float*)v)[1], ((float*)v)[2]);
	else if (t == TypeComplex)
		var_temp += format("(%f %f)", *(float*)v, ((float*)v)[1]);
	else
		var_temp += "-------";
}

void try_write_element(EffectParam *p, sClassElement *e, char *v)
{
	p->name = e->Name;
	p->type = e->Type->Name;
	p->value = "";
	if (e->Type->IsArray){
		p->value += "[";
		for (int i=0;i<e->Type->ArrayLength;i++){
			if (i > 0)
				p->value += " ";
			try_write_primitive_element(p->value, e->Type->SubType, &v[e->Offset + i * e->Type->SubType->Size]);
		}
		p->value += "]";
	}else if (e->Type->IsSuperArray){
		DynamicArray *a = (DynamicArray*)&v[e->Offset];
		p->value += format("[%d ", a->num);
		for (int i=0;i<a->num;i++){
			if (i > 0)
				p->value += " ";
			try_write_primitive_element(p->value, e->Type->SubType, &(((char*)a->data)[i * e->Type->SubType->Size]));
		}
		p->value += "]";
	}else
		try_write_primitive_element(p->value, e->Type, &v[e->Offset]);
}

string get_next(const string &var_temp, int &pos)
{
	int start = pos;
	for (int i=pos;i<var_temp.num;i++){
		if ((var_temp[i] != ' ') && (var_temp[i] != ']') && (var_temp[i] != ')') && (var_temp[i] != '[') && (var_temp[i] != '(')){
			start = i;
			break;
		}
	}
	for (int i=start;i<var_temp.num;i++){
		if ((var_temp[i] == ' ') || (var_temp[i] == ']') || (var_temp[i] == ')') || (var_temp[i] == '[') || (var_temp[i] == '(')){
			pos = i + 1;
			return var_temp.substr(start, i - start);
			break;
		}
	}
	return var_temp.substr(start, -1);
}

void try_read_primitive_element(const string &var_temp, int &pos, sType *t, char *v)
{
	if (t == TypeInt)
		*(int*)v = s2i(get_next(var_temp, pos));
	else if (t == TypeChar)
		*(char*)v = s2i(get_next(var_temp, pos));
	else if (t == TypeFloat)
		*(float*)v = s2f(get_next(var_temp, pos));
	else if (t == TypeComplex){
		((complex*)v)->x = s2f(get_next(var_temp, pos));
		((complex*)v)->y = s2f(get_next(var_temp, pos));
	}else if (t == TypeVector){
		((vector*)v)->x = s2f(get_next(var_temp, pos));
		((vector*)v)->y = s2f(get_next(var_temp, pos));
		((vector*)v)->z = s2f(get_next(var_temp, pos));
	}else if (t == TypeBool)
		*(bool*)v = (get_next(var_temp, pos) == "true");
}

void try_read_element(EffectParam &p, sClassElement *e, char *v)
{
	int pos = 0;
	if (e->Type->IsArray){
		for (int i=0;i<e->Type->ArrayLength;i++)
			try_read_primitive_element(p.value, pos, e->Type->SubType, &v[e->Offset + i * e->Type->SubType->Size]);
	}else if (e->Type->IsSuperArray){
		DynamicArray *a = (DynamicArray*)&v[e->Offset];
		int num = s2i(get_next(p.value, pos));
		a->resize(num);
		for (int i=0;i<num;i++)
			try_read_primitive_element(p.value, pos, e->Type->SubType, &(((char*)a->data)[i * e->Type->SubType->Size]));
	}else
		try_read_primitive_element(p.value, pos, e->Type, &v[e->Offset]);
}

void PluginManager::ExportPluginData(Effect &fx)
{
	msg_db_r("ExportPluginData", 1);
	fx.param.clear();
	for (int i=0;i<cur_plugin->s->pre_script->RootOfAllEvil.Var.num;i++){
		sType *t = cur_plugin->s->pre_script->RootOfAllEvil.Var[i].Type;
		if (strcmp(t->Name, "PluginData") == 0){
			fx.param.resize(t->Element.num);
			for (int j=0;j<t->Element.num;j++){
				sClassElement *e = &t->Element[j];
				try_write_element(&fx.param[j], e, cur_plugin->s->g_var[i]);
			}
			break;
		}
	}
	msg_db_l(1);
}

void PluginManager::ImportPluginData(Effect &fx)
{
	msg_db_r("ImportPluginData", 1);
	for (int i=0;i<cur_plugin->s->pre_script->RootOfAllEvil.Var.num;i++){
		sType *t = cur_plugin->s->pre_script->RootOfAllEvil.Var[i].Type;
		if (strcmp(t->Name, "PluginData") == 0){
			for (int j=0;j<t->Element.num;j++){
				sClassElement *e = &t->Element[j];
				for (int k=0;k<fx.param.num;k++)
					if (string(e->Name) == fx.param[k].name)
						try_read_element(fx.param[k], e, cur_plugin->s->g_var[i]);
			}
			break;
		}
	}
	msg_db_l(1);
}

void PluginManager::WritePluginDataToFile(const string &name)
{
	msg_db_r("WritePluginDataToFile", 1);
	Effect fx;
	ExportPluginData(fx);
	CFile *f = CreateFile(HuiAppDirectory + "Plugins/Favorites/" + basename(cur_plugin->s->pre_script->Filename) + "___" + name);
	f->WriteInt(0);
	f->WriteInt(0);
	f->WriteComment("// Data");
	f->WriteInt(fx.param.num);
	for (int i=0;i<fx.param.num;i++){
		f->WriteStr(fx.param[i].name);
		f->WriteStr(fx.param[i].type);
		f->WriteStr(fx.param[i].value);
	}
	fx.param.clear();
	f->WriteStr("#");
	FileClose(f);
	msg_db_l(1);
}

void PluginManager::LoadPluginDataFromFile(const string &name)
{
	msg_db_r("LoadPluginDataFromFile", 1);
	CFile *f = OpenFile(HuiAppDirectory + "Plugins/Favorites/" + basename(cur_plugin->s->pre_script->Filename) + "___" + name);
	if (!f){
		msg_db_l(1);
		return;
	}
	Effect fx;

	f->ReadInt();
	f->ReadInt();
	f->ReadComment();
	int num = f->ReadInt();
	fx.param.resize(num);
	for (int i=0;i<num;i++){
		fx.param[i].name = f->ReadStr();
		fx.param[i].type = f->ReadStr();
		fx.param[i].value = f->ReadStr();
	}
	ImportPluginData(fx);
	fx.param.clear();

	FileClose(f);
	msg_db_l(1);
}

void PluginManager::PluginPreview()
{
	Effect fx;
	fx.filename = cur_plugin->s->pre_script->Filename.substr(cur_plugin->s->pre_script->Filename.find("All - ") + 6, -1);
	fx.filename = fx.filename.substr(0, fx.filename.num - 5);
	//msg_write(fx.filename);
	ExportPluginData(fx);
	tsunami->renderer->effect = &fx;

	//CHuiWindow *dlg = HuiCurWindow;
	//dlg->Hide(true);
	//HuiCurWindow = MainWin;


	tsunami->progress->StartCancelable(_("Vorschau"), 0);
	tsunami->output->Play(tsunami->cur_audio, false);

	while(tsunami->output->IsPlaying()){
		tsunami->output->Update();
		HuiSleep(10);
		HuiDoSingleMainLoop();
		int pos = tsunami->output->GetPos(tsunami->cur_audio);
		tsunami->progress->Set(_("Vorschau"), (float)(pos - tsunami->cur_audio->selection_start) / tsunami->cur_audio->selection_length);
		if (tsunami->progress->IsCancelled()){
			tsunami->output->Stop();
			break;
		}
	}
	tsunami->progress->End();
	//dlg->Hide(false);
	//HuiCurWindow = dlg;


	tsunami->renderer->effect = NULL;
	ImportPluginData(fx);
	fx.param.clear();
}

// always push the script... even if an error occurred
bool PluginManager::LoadAndCompilePlugin(const string &filename)
{
	msg_db_r("LoadAndCompilePlugin", 1);

	//msg_write(filename);

	foreach(plugin, p){
		if (filename == p.filename){
			PushCurPlugin(p.s);
			return true;
		}
	}

	Plugin s;
	s.filename = filename;
	s.index = plugin.num;

	//InitPluginData();

	// linking would kill type information already defined in api.kaba...
	if (plugin.num == 0)
		LinkAppScriptData();

	// load + compile
	s.s = LoadScript(filename);

	// NULL if error...
	s.f_reset = (hui_callback*)s.s->MatchFunction("ResetData", "void", 0);
	s.f_data2dialog = (hui_callback*)s.s->MatchFunction("DataToDialog", "void", 0);
	s.f_configure = (hui_callback*)s.s->MatchFunction("Configure", "void", 0);

	plugin.add(s);
	PushCurPlugin(plugin.back().s);

	msg_db_l(1);
	return !s.s->Error;
}

void PluginManager::PluginProcessTrack(CScript *s, Track *t, int pos, int length)
{
	//tsunami->cur_audio->history->ChangeBegin();
	//HistoryOpStartEditBuffer(t, s_start, s_end);
	BufferBox buf = t->GetBuffers(pos, length);
	s->ExecuteScriptFunction("ProcessTrack", &buf, t);
	//t->UpdatePeaks();
	//cur_audio->history->ChangeEnd();
}

void PluginManager::ExecutePlugin(const string &filename)
{
	msg_db_r("ExecutePlugin", 1);

	if (LoadAndCompilePlugin(filename)){
		CScript *s = cur_plugin->s;

		// run
//		cur_audio->history->ChangeBegin();
		PluginResetData();
		if (PluginConfigure(false, true)){
			if (s->MatchFunction("ProcessTrack", "void", 2, "BufferBox", "Track")){
				if (tsunami->cur_audio->used){
//					tsunami->cur_audio->history->ChangeBegin();
					foreach(tsunami->cur_audio->track, t)
						if (t.is_selected)
							PluginProcessTrack(s, &t, tsunami->cur_audio->selection_start, tsunami->cur_audio->selection_length);
//					cur_audio->history->ChangeEnd();
				}else{
					tsunami->log->Error(_("Plugin kann nicht f&ur eine leere Audiodatei ausgef&uhrt werden"));
				}
			}else if (s->MatchFunction("main", "void", 1, "AudioFile*")){
				if (tsunami->cur_audio->used)
					s->ExecuteScriptFunction("main", tsunami->cur_audio);
				else
					tsunami->log->Error(_("Plugin kann nicht f&ur eine leere Audiodatei ausgef&uhrt werden"));
			}else{
				s->WaitingMode = WaitingModeFirst;
				s->Execute();
			}
		}
//		cur_audio->history->ChangeEnd();

		// data changed?
		FinishPluginData();
	}else{
		string fn = filename;
		if (cur_plugin->s->pre_script)
			fn = cur_plugin->s->pre_script->Filename;
		tsunami->log->Error(format(_("Fehler in  Script-Datei: \"%s\"\n%s\n%s"), fn.c_str(), cur_plugin->s->ErrorMsgExt[0].c_str(), cur_plugin->s->ErrorMsgExt[1].c_str()));
	}
	PopCurPlugin();

	msg_db_l(1);
}


void PluginManager::FindAndExecutePlugin()
{
	msg_db_r("ExecutePlugin", 1);


	if (HuiFileDialogOpen(tsunami, _("Plugin-Script w&ahlen"), HuiAppDirectoryStatic + "Plugins/", _("Script (*.kaba)"), "*.kaba")){
		ExecutePlugin(HuiFilename);
	}
	msg_db_l(1);
}


bool PluginManager::LoadAndCompileEffect(const string &filename)
{
	string _filename_ = HuiAppDirectoryStatic + "Plugins/All - " + filename + ".kaba";

	return LoadAndCompilePlugin(_filename_);
}

void PluginManager::ApplyEffects(BufferBox &buf, Track *t, Effect *fx)
{
	msg_db_r("ApplyEffects", 1);

	if (LoadAndCompileEffect(fx->filename)){
		// run
		PluginResetData();
		ImportPluginData(*fx);
		if (cur_plugin->s->MatchFunction("ProcessTrack", "void", 2, "BufferBox", "Track")){
			cur_plugin->s->ExecuteScriptFunction("ProcessTrack", &buf, t);
		}
		t->root->UpdateSelection();
	}else{
		if (cur_plugin->s)
			tsunami->log->Error(format(_("Beim Anwenden eines Effekt-Scripts (%s: %s %s)"), fx->filename.c_str(), cur_plugin->s->ErrorMsgExt[0].c_str(), cur_plugin->s->ErrorMsgExt[1].c_str()));
		else
			tsunami->log->Error(format(_("Beim Anwenden eines Effekt-Scripts (%s: Datei nicht ladbar)"), fx->filename.c_str()));
	}
	PopCurPlugin();

	msg_db_l(1);
}
