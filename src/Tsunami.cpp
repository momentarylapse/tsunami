/*
 * Tsunami.cpp
 *
 *  Created on: 21.03.2012
 *      Author: michi
 */

#include "lib/hui/hui.h"
#include "Tsunami.h"
#include "View/Dialog/NewDialog.h"


Tsunami *tsunami = NULL;
extern string AppName;

Tsunami::Tsunami(Array<string> arg) :
	CHuiWindow(AppName, -1, -1, 800, 600, NULL, false, HuiWinModeResizable | HuiWinModeControls, true)
{
	tsunami = this;

	// configuration
/*	ChosenOutputDevice = HuiConfigReadStr("ChosenOutputDevice", "");
	ChosenInputDevice = HuiConfigReadStr("ChosenInputDevice", "");
	CapturePlaybackDelay = HuiConfigReadFloat("CapturePlaybackDelay", 80.0f);*/

	int width = HuiConfigReadInt("Width", 1024);
	int height = HuiConfigReadInt("Height", 768);
	bool maximized = HuiConfigReadBool("Maximized", true);
/*	OggQuality = HuiConfigReadFloat("OggQuality", 0.5f);
	Preview.volume = HuiConfigReadFloat("Volume", 1.0f);
	DrawingWidth = width;*/

	//HuiAddKeyCode("insert_added", KEY_RETURN);
	//HuiAddKeyCode("remove_added", KEY_BACKSPACE);
	HuiAddKeyCode("jump_other_file", KEY_TAB);

	HuiAddCommandM("new", "hui:new", KEY_N + KEY_CONTROL, this, (void(HuiEventHandler::*)())&Tsunami::OnNew);
	HuiAddCommandM("open", "hui:open", KEY_O + KEY_CONTROL, this, (void(HuiEventHandler::*)())&Tsunami::OnOpen);
	HuiAddCommandM("save", "hui:save", KEY_S + KEY_CONTROL, this, (void(HuiEventHandler::*)())&Tsunami::OnSave);
	HuiAddCommandM("save_as", "hui:save-as", KEY_S + KEY_CONTROL + KEY_SHIFT, this, (void(HuiEventHandler::*)())&Tsunami::OnSaveAs);
	HuiAddCommandM("copy", "hui:copy", KEY_C + KEY_CONTROL, this, (void(HuiEventHandler::*)())&Tsunami::OnCopy);
	HuiAddCommandM("paste", "hui:paste", KEY_V + KEY_CONTROL, this, (void(HuiEventHandler::*)())&Tsunami::OnPaste);
	HuiAddCommandM("delete", "", -1, this, (void(HuiEventHandler::*)())&Tsunami::OnDelete);
	HuiAddCommandM("export_selection", "", KEY_X + KEY_CONTROL, this, (void(HuiEventHandler::*)())&Tsunami::OnExport);
	HuiAddCommandM("undo", "hui:undo", KEY_Z + KEY_CONTROL, this, (void(HuiEventHandler::*)())&Tsunami::OnUndo);
	HuiAddCommandM("redo", "hui:redo", KEY_Z + KEY_SHIFT + KEY_CONTROL, this, (void(HuiEventHandler::*)())&Tsunami::OnRedo);
	HuiAddCommandM("add_track", "", -1, this, (void(HuiEventHandler::*)())&Tsunami::OnAddTrack);
	HuiAddCommandM("add_time_track", "", -1, this, (void(HuiEventHandler::*)())&Tsunami::OnAddTimeTrack);
	HuiAddCommandM("delete_track", "", -1, this, (void(HuiEventHandler::*)())&Tsunami::OnDeleteTrack);
	HuiAddCommandM("insert_added", "", KEY_I + KEY_CONTROL, this, (void(HuiEventHandler::*)())&Tsunami::OnInsertAdded);
	HuiAddCommandM("remove_added", "", -1, this, (void(HuiEventHandler::*)())&Tsunami::OnRemoveAdded);
	HuiAddCommandM("track_import", "", -1, this, (void(HuiEventHandler::*)())&Tsunami::OnTrackImport);
	HuiAddCommandM("sub_import", "", -1, this, (void(HuiEventHandler::*)())&Tsunami::OnSubImport);
	HuiAddCommandM("wave_properties", "", KEY_F4, this, (void(HuiEventHandler::*)())&Tsunami::OnAudioProperties);
	HuiAddCommandM("track_properties", "", -1, this, (void(HuiEventHandler::*)())&Tsunami::OnTrackProperties);
	HuiAddCommandM("sub_properties", "", -1, this, (void(HuiEventHandler::*)())&Tsunami::OnSubProperties);
	HuiAddCommandM("settings", "", -1, this, (void(HuiEventHandler::*)())&Tsunami::OnSettings);
	HuiAddCommandM("close_file", "hui:close", KEY_W + KEY_CONTROL, this, (void(HuiEventHandler::*)())&Tsunami::OnCloseFile);
	HuiAddCommandM("play", "", -1, this, (void(HuiEventHandler::*)())&Tsunami::OnPlay);
	HuiAddCommandM("play_loop", "", -1, this, (void(HuiEventHandler::*)())&Tsunami::OnPlayLoop);
	HuiAddCommandM("stop", "", -1, this, (void(HuiEventHandler::*)())&Tsunami::OnStop);
	HuiAddCommandM("record", "", -1, this, (void(HuiEventHandler::*)())&Tsunami::OnRecord);
	HuiAddCommandM("show_log", "", -1, this, (void(HuiEventHandler::*)())&Tsunami::OnShowLog);
	HuiAddCommandM("about", "", -1, this, (void(HuiEventHandler::*)())&Tsunami::OnAbout);
	HuiAddCommandM("run_plugin", "hui:execute", KEY_RETURN + KEY_SHIFT, this, (void(HuiEventHandler::*)())&Tsunami::OnFindAndExecutePlugin);
	HuiAddCommandM("exit", "hui:quit", KEY_Q + KEY_CONTROL, this, (void(HuiEventHandler::*)())&Tsunami::OnExit);


	// create the window
	SetSize(width, height);
	SetBorderWidth(0);
	AllowEvents("key");
	ToolbarSetByID("toolbar");
	//ToolbarConfigure(false, true);

	SetMenu(HuiCreateResourceMenu("menu"));
	//ToolBarConfigure(true, true);
	SetMaximized(maximized);

	// events
	EventM("hui:close", this, (void(HuiEventHandler::*)())&Tsunami::OnExit);

	/*AddPluginsToMenu();

	input_timer = HuiCreateTimer();

	InitHistory();*/
	audio[0] = new AudioFile();
	audio[1] = new AudioFile();

	cur_audio = audio[0];

	storage = new Storage();

	view = new AudioView();

	progress = new Progress;

	Subscribe(view);
	Subscribe(audio[0]);
	Subscribe(audio[1]);

	UpdateMenu();

	/*PreviewInit();

	force_rendering = true;

	HuiSetIdleFunction(&IdleFunction);

	if (arg.num > 1)
		LoadFromFile(audio[0], arg[1]);

	UpdateMenu();*/
	Update();
}

Tsunami::~Tsunami()
{
}


int Tsunami::Run()
{
	return HuiRun();
}

void Tsunami::ForceRedraw()
{
	view->ForceRedraw();
}



void Tsunami::OnAbout()
{
	HuiAboutBox(this);
}



void Tsunami::OnAddTrack()
{
	cur_audio->AddEmptyTrack();
}

void Tsunami::OnAddTimeTrack()
{
}

void Tsunami::OnCloseFile()
{
	cur_audio->Reset();
}

void Tsunami::OnStop()
{
}

void Tsunami::LoadKeyCodes()
{
}

void Tsunami::OnAudioProperties()
{
}

void Tsunami::OnShowLog()
{
}

void Tsunami::OnDeleteTrack()
{
}

void Tsunami::OnUndo()
{
	cur_audio->action_manager->Undo();
}

void Tsunami::OnRedo()
{
	cur_audio->action_manager->Redo();
}

void Tsunami::OnSendBugReport()
{
}

void Tsunami::IdleFunction()
{
}

bool Tsunami::AllowTermination(AudioFile *a)
{
	return true;
}

void Tsunami::OnCopy()
{
}

void Tsunami::OnExport()
{
}

void Tsunami::OnFindAndExecutePlugin()
{
}

void Tsunami::OnDelete()
{
}

void Tsunami::OnSubImport()
{
}

void Tsunami::OnCommand(const string & id)
{
}

void Tsunami::OnSettings()
{
}

void Tsunami::OnTrackImport()
{
}

bool Tsunami::HandleArguments(Array<string> arg)
{
	return false;
}

void Tsunami::OnTrackProperties()
{
}

void Tsunami::OnRemoveAdded()
{
}

void Tsunami::OnSubProperties()
{
}

void Tsunami::OnPlayLoop()
{
}

void Tsunami::OnPlay()
{
}

void Tsunami::OnPaste()
{
}

void Tsunami::OnInsertAdded()
{
}

void Tsunami::OnRecord()
{
}

void Tsunami::UpdateMenu()
{
	msg_db_r("UpdateMenu", 1);
// menu / toolbar
	// edit
	Enable("undo", cur_audio->action_manager->Undoable());
	Enable("redo", cur_audio->action_manager->Redoable());
	/*m->EnableItem("select_all", cur_audio->used);
	m->EnableItem("select_nothing", cur_audio->used);
	m->EnableItem("copy", cur_audio->used);
	MainWin->Enable("copy", cur_audio->used);
	m->EnableItem("delete", cur_audio->used);
	MainWin->Enable("delete", cur_audio->used);
	// file
	m->EnableItem("save", cur_audio->used);
	MainWin->Enable("save", cur_audio->used);
	m->EnableItem("save_as", cur_audio->used);
	m->EnableItem("cut_out", cur_audio->used);
	m->EnableItem("close_file", cur_audio->used);
	m->EnableItem("wave_properties", cur_audio->used);
	// track
	m->EnableItem("track_import", cur_audio->used);
	m->EnableItem("add_track", cur_audio->used);
	m->EnableItem("delete_track", GetCurTrack());
	m->EnableItem("track_properties", GetCurTrack());
	// sub
	m->EnableItem("sub_import", GetCurTrack());
	//m->EnableItem(HMM_INSERT_ADDED, cur_sub);
	//m->EnableItem(HMM_REMOVE_ADDED, cur_sub);
	//m->EnableItem(HMM_SUB_PROPERTIES, cur_sub);
	// view
	m->CheckItem("view_temp_file", ShowTempFile);
	m->CheckItem("view_mono", ShowMono);
	m->CheckItem("view_grid", ShowGrid);
	m->EnableItem("zoom_in", cur_audio->used);
	m->EnableItem("zoom_out", cur_audio->used);
	m->EnableItem("view_optimal", cur_audio->used);
	m->EnableItem("view_samples", false);//cur_audio->used);
	// sound
	m->EnableItem("play", cur_audio->used);
	m->EnableItem("stop", Preview.playing);
	m->CheckItem("play_loop", PreviewPlayLoop);
	MainWin->Enable("play", cur_audio->used);
	MainWin->Enable("stop", Preview.playing);*/
	msg_db_l(1);
}


void Tsunami::OnUpdate(Observable *o)
{
	UpdateMenu();
}


void Tsunami::OnExit()
{
	if (AllowTermination())
		HuiEnd();
}


void Tsunami::OnNew()
{
	NewDialog *d = new NewDialog(tsunami, false, cur_audio);
	d->Update();
	HuiWaitTillWindowClosed(d);
}


void Tsunami::OnOpen()
{
	if (storage->AskOpen(this))
		storage->Load(cur_audio, HuiFilename);
}


void Tsunami::OnSave()
{
}


void Tsunami::OnSaveAs()
{
}


void Tsunami::Log(int type, const string &message)
{
	msg_error(message);
}
