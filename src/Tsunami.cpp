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
	CurrentDirectory = HuiConfigReadStr("CurrentDirectory", "");
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
	/*HuiAddCommand("copy", "hui:copy", KEY_C + KEY_CONTROL, &Copy);
	HuiAddCommand("paste", "hui:paste", KEY_V + KEY_CONTROL, &Paste);
	HuiAddCommand("delete", "", -1, &Delete);
	HuiAddCommand("export_selection", "", KEY_X + KEY_CONTROL, &Export);
	HuiAddCommand("undo", "hui:undo", KEY_Z + KEY_CONTROL, &Undo);
	HuiAddCommand("redo", "hui:redo", KEY_Z + KEY_SHIFT + KEY_CONTROL, &Redo);
	HuiAddCommand("select_none", "", -1, &OnSelectNone);
	HuiAddCommand("select_all", "", KEY_A + KEY_CONTROL, &OnSelectAll);
	HuiAddCommand("select_nothing", "", -1, &OnSelectNothing);
	HuiAddCommand("add_track", "", -1, &OnAddTrack);
	HuiAddCommand("add_time_track", "", -1, &OnAddTimeTrack);
	HuiAddCommand("delete_track", "", -1, &OnDeleteTrack);
	HuiAddCommand("insert_added", "", KEY_I + KEY_CONTROL, &OnInsertAdded);
	HuiAddCommand("remove_added", "", -1, &OnRemoveAdded);
	HuiAddCommand("track_import", "", -1, &OnTrackImport);
	HuiAddCommand("sub_import", "", -1, &OnSubImport);
	HuiAddCommand("wave_properties", "", KEY_F4, &OnWaveProperties);
	HuiAddCommand("track_properties", "", -1, &OnTrackProperties);
	HuiAddCommand("sub_properties", "", -1, &OnSubProperties);
	HuiAddCommand("settings", "", -1, &OnSettings);
	HuiAddCommand("close_file", "hui:close", KEY_W + KEY_CONTROL, &OnCloseFile);
	HuiAddCommand("play", "", -1, &OnPlay);
	HuiAddCommand("play_loop", "", -1, &OnPlayLoop);
	HuiAddCommand("stop", "", -1, &OnStop);
	HuiAddCommand("record", "", -1, &Record);
	HuiAddCommand("show_log", "", -1, &OnShowLog);
	HuiAddCommand("about", "", -1, &OnAbout);
	HuiAddCommand("view_temp_file", "", -1, &OnViewTempFile);
	HuiAddCommand("view_mono", "", -1, &OnViewMono);
	HuiAddCommand("view_grid", "", -1, &OnViewGrid);
	HuiAddCommand("view_optimal", "", -1, &OnViewOptimal);
	HuiAddCommand("zoom_in", "", -1, &OnZoomIn);
	HuiAddCommand("zoom_out", "", -1, &OnZoomOut);
	HuiAddCommand("jump_other_file", "", -1, &OnJumpOtherFile);
	HuiAddCommand("run_plugin", "hui:execute", KEY_RETURN + KEY_SHIFT, &FindAndExecutePlugin);
	HuiAddCommand("exit", "hui:quit", KEY_Q + KEY_CONTROL, &OnExit);*/


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
	EventM("hui:close", this, (void(HuiEventHandler::*)())&Tsunami::OnClose);

	/*AddPluginsToMenu();

	input_timer = HuiCreateTimer();

	InitHistory();*/
	audio[0] = new AudioFile();
	audio[1] = new AudioFile();

	cur_audio = audio[0];

	storage = new Storage();

	view = new AudioView();

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
}



void Tsunami::UpdateMenu()
{
}


void Tsunami::OnUpdate(Observable *o)
{
}


void Tsunami::OnClose()
{
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
}


void Tsunami::OnSave()
{
}


void Tsunami::OnSaveAs()
{
}



