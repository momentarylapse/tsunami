/*
 * Tsunami.cpp
 *
 *  Created on: 21.03.2012
 *      Author: michi
 */

#include "lib/hui/hui.h"
#include "Tsunami.h"
#include "View/Dialog/NewDialog.h"
#include "View/Dialog/CaptureDialog.h"


Tsunami *tsunami = NULL;
extern string AppName;

Tsunami::Tsunami(Array<string> arg) :
	CHuiWindow(AppName, -1, -1, 800, 600, NULL, false, HuiWinModeResizable | HuiWinModeControls, true)
{
	tsunami = this;

	progress = new Progress;
	log = new Log;

	output = new AudioOutput;
	input = new AudioInput;
	renderer = new AudioRenderer;


	int width = HuiConfigReadInt("Window.Width", 800);
	int height = HuiConfigReadInt("Window.Height", 600);
	bool maximized = HuiConfigReadBool("Window.Maximized", true);
/*	OggQuality = HuiConfigReadFloat("OggQuality", 0.5f); // -> StorageOgg*/

	//HuiAddKeyCode("insert_added", KEY_RETURN);
	//HuiAddKeyCode("remove_added", KEY_BACKSPACE);
	HuiAddKeyCode("jump_other_file", KEY_TAB);

	HuiAddCommandM("new", "hui:new", KEY_N + KEY_CONTROL, this, (void(HuiEventHandler::*)())&Tsunami::OnNew);
	HuiAddCommandM("open", "hui:open", KEY_O + KEY_CONTROL, this, (void(HuiEventHandler::*)())&Tsunami::OnOpen);
	HuiAddCommandM("save", "hui:save", KEY_S + KEY_CONTROL, this, (void(HuiEventHandler::*)())&Tsunami::OnSave);
	HuiAddCommandM("save_as", "hui:save-as", KEY_S + KEY_CONTROL + KEY_SHIFT, this, (void(HuiEventHandler::*)())&Tsunami::OnSaveAs);
	HuiAddCommandM("copy", "hui:copy", KEY_C + KEY_CONTROL, this, (void(HuiEventHandler::*)())&Tsunami::OnCopy);
	HuiAddCommandM("paste", "hui:paste", KEY_V + KEY_CONTROL, this, (void(HuiEventHandler::*)())&Tsunami::OnPaste);
	HuiAddCommandM("delete", "hui:delete", -1, this, (void(HuiEventHandler::*)())&Tsunami::OnDelete);
	HuiAddCommandM("export_selection", "", KEY_X + KEY_CONTROL, this, (void(HuiEventHandler::*)())&Tsunami::OnExport);
	HuiAddCommandM("undo", "hui:undo", KEY_Z + KEY_CONTROL, this, (void(HuiEventHandler::*)())&Tsunami::OnUndo);
	HuiAddCommandM("redo", "hui:redo", KEY_Z + KEY_SHIFT + KEY_CONTROL, this, (void(HuiEventHandler::*)())&Tsunami::OnRedo);
	HuiAddCommandM("add_track", "hui:add", -1, this, (void(HuiEventHandler::*)())&Tsunami::OnAddTrack);
	HuiAddCommandM("add_time_track", "hui:add", -1, this, (void(HuiEventHandler::*)())&Tsunami::OnAddTimeTrack);
	HuiAddCommandM("delete_track", "hui:delete", -1, this, (void(HuiEventHandler::*)())&Tsunami::OnDeleteTrack);
	HuiAddCommandM("insert_added", "", KEY_I + KEY_CONTROL, this, (void(HuiEventHandler::*)())&Tsunami::OnInsertAdded);
	HuiAddCommandM("remove_added", "", -1, this, (void(HuiEventHandler::*)())&Tsunami::OnRemoveAdded);
	HuiAddCommandM("track_import", "", -1, this, (void(HuiEventHandler::*)())&Tsunami::OnTrackImport);
	HuiAddCommandM("sub_import", "", -1, this, (void(HuiEventHandler::*)())&Tsunami::OnSubImport);
	HuiAddCommandM("wave_properties", "", KEY_F4, this, (void(HuiEventHandler::*)())&Tsunami::OnAudioProperties);
	HuiAddCommandM("track_properties", "", -1, this, (void(HuiEventHandler::*)())&Tsunami::OnTrackProperties);
	HuiAddCommandM("sub_properties", "", -1, this, (void(HuiEventHandler::*)())&Tsunami::OnSubProperties);
	HuiAddCommandM("settings", "", -1, this, (void(HuiEventHandler::*)())&Tsunami::OnSettings);
	HuiAddCommandM("close_file", "hui:close", KEY_W + KEY_CONTROL, this, (void(HuiEventHandler::*)())&Tsunami::OnCloseFile);
	HuiAddCommandM("play", "hui:media-play", -1, this, (void(HuiEventHandler::*)())&Tsunami::OnPlay);
	HuiAddCommandM("play_loop", "", -1, this, (void(HuiEventHandler::*)())&Tsunami::OnPlayLoop);
	HuiAddCommandM("stop", "hui:media-stop", -1, this, (void(HuiEventHandler::*)())&Tsunami::OnStop);
	HuiAddCommandM("record", "hui:media-record", -1, this, (void(HuiEventHandler::*)())&Tsunami::OnRecord);
	HuiAddCommandM("show_log", "", -1, this, (void(HuiEventHandler::*)())&Tsunami::OnShowLog);
	HuiAddCommandM("about", "", -1, this, (void(HuiEventHandler::*)())&Tsunami::OnAbout);
	HuiAddCommandM("run_plugin", "hui:execute", KEY_RETURN + KEY_SHIFT, this, (void(HuiEventHandler::*)())&Tsunami::OnFindAndExecutePlugin);
	HuiAddCommandM("exit", "hui:quit", KEY_Q + KEY_CONTROL, this, (void(HuiEventHandler::*)())&Tsunami::OnExit);


	// create the window
	SetSize(width, height);
	SetBorderWidth(0);
	AddControlTable("", 0, 0, 1, 2, "main_table");
	SetTarget("main_table", 0);
	SetBorderWidth(8);
	AddControlTable("", 0, 1, 8, 1, "audio_table");
	SetTarget("audio_table", 0);
	AddButton("", 0, 0, 0, 0, "play");
	AddButton("", 1, 0, 0, 0, "stop");
	AddDrawingArea("!width=100", 2, 0, 0, 0, "peaks");
	peak_meter = new PeakMeter(this, "peaks");
	AddSlider("!width=100", 3, 0, 0, 0, "volume_slider");
	AddSpinButton("!width=50\\0\\0\\100", 4, 0, 0, 0, "volume");
	AddText("!width=20\\%", 5, 0, 0, 0, "label_percent");
	volume_slider = new Slider(this, "volume_slider", "volume", 0, 1, 100, (void(HuiEventHandler::*)())&Tsunami::OnVolume, output->GetVolume());
	AddButton("", 6, 0, 0, 0, "record");
	AllowEvents("key");
	ToolbarSetByID("toolbar");
	//ToolbarConfigure(false, true);

	SetMenu(HuiCreateResourceMenu("menu"));
	//ToolBarConfigure(true, true);
	SetMaximized(maximized);

	// events
	EventM("hui:close", this, (void(HuiEventHandler::*)())&Tsunami::OnExit);

	plugins = new PluginManager;
	plugins->AddPluginsToMenu();

	audio[0] = new AudioFile;
	audio[1] = new AudioFile;

	cur_audio = audio[0];

	storage = new Storage;

	view = new AudioView;

	Subscribe(view);
	Subscribe(audio[0]);
	Subscribe(audio[1]);
	Subscribe(output);

	UpdateMenu();

	HandleArguments(arg);

	Update();
}

Tsunami::~Tsunami()
{
	irect r = GetOuteriorDesired();
	HuiConfigWriteInt("Window.Width", r.x2 - r.x1);
	HuiConfigWriteInt("Window.Height", r.y2 - r.y1);
	HuiConfigWriteInt("Window.X", r.x1);
	HuiConfigWriteInt("Window.Y", r.y1);
	HuiConfigWriteBool("Window.Maximized", IsMaximized());

	delete(plugins);
	delete(storage);
	delete(view);
	delete(output);
	delete(input);
	HuiEnd();
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
	Track *t = cur_audio->AddTimeTrack();
	if (t){
		// example data...
		BarCollection c;
		c.pos = 0;
		TimeBar b;
		b.num_beats = 4;
		b.length = 90000;
		c.bar.add(b);
		c.bar.add(b);
		c.bar.add(b);
		c.bar.add(b);
		c.bar.add(b);
		c.Update();
		t->bar_col.add(c);
	}
}

void Tsunami::OnDeleteTrack()
{
}

void Tsunami::OnCloseFile()
{
	cur_audio->Reset();
}

void Tsunami::LoadKeyCodes()
{
}

void Tsunami::OnAudioProperties()
{
	view->ExecuteAudioDialog(this, cur_audio);
}

void Tsunami::OnTrackProperties()
{
	view->ExecuteTrackDialog(this, GetCurTrack());
}

void Tsunami::OnSubProperties()
{
	view->ExecuteSubDialog(this, GetCurSub());
}

void Tsunami::OnShowLog()
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

bool Tsunami::AllowTermination(AudioFile *a)
{
	return true;
}

void Tsunami::OnCopy()
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
	if (arg.num > 1)
		return storage->Load(cur_audio, arg[1]);
	return false;
}

void Tsunami::OnRemoveAdded()
{
}

void Tsunami::OnPlayLoop()
{
	output->PlayLoop = !output->PlayLoop;
	UpdateMenu();
}

void Tsunami::OnPlay()
{
	output->Play(cur_audio, output->PlayLoop);
}

void Tsunami::OnStop()
{
	output->Stop();
}

void Tsunami::OnVolume()
{
	output->SetVolume(volume_slider->Get());
}

void Tsunami::OnPaste()
{
}

void Tsunami::OnInsertAdded()
{
}

void Tsunami::OnRecord()
{
	CaptureDialog *dlg = new CaptureDialog(this, false, cur_audio);
	dlg->Update();
	HuiWaitTillWindowClosed(dlg);
}


string title_filename(const string &filename)
{
	if (filename.num > 0)
		return basename(filename);// + " (" + dirname(filename) + ")";
	return _("Unbenannt");
}

Track *Tsunami::GetCurTrack()
{	return cur_audio->GetCurTrack();	}

Track *Tsunami::GetCurSub()
{	return cur_audio->GetCurSub();	}

void Tsunami::UpdateMenu()
{
	msg_db_r("UpdateMenu", 1);
// menu / toolbar
	// edit
	Enable("undo", cur_audio->action_manager->Undoable());
	Enable("redo", cur_audio->action_manager->Redoable());
	Enable("copy", cur_audio->used);
	Enable("delete", cur_audio->used);
	Enable("resize", false); // deprecated
	// file
	Enable("save", cur_audio->used);
	Enable("save", cur_audio->used);
	Enable("save_as", cur_audio->used);
	Enable("cut_out", cur_audio->used);
	Enable("close_file", cur_audio->used);
	Enable("export_selection", cur_audio->used);
	Enable("wave_properties", cur_audio->used);
	// track
	Enable("track_import", cur_audio->used);
	Enable("add_track", cur_audio->used);
	Enable("add_time_track", cur_audio->used);
	Enable("delete_track", GetCurTrack());
	Enable("track_properties", GetCurTrack());
	// sub
	Enable("sub_import", GetCurTrack());
	Enable("insert_added", GetCurSub());
	Enable("remove_added", GetCurSub());
	Enable("sub_properties", GetCurSub());
	// sound
	Enable("play", cur_audio->used);
	Enable("stop", output->IsPlaying());
	Check("play_loop", output->PlayLoop);
	Enable("play", cur_audio->used);
	Enable("stop", output->IsPlaying());


	if (cur_audio->used){
		string title = title_filename(cur_audio->filename) + " - " + AppName;
		if (!cur_audio->action_manager->IsSave())
			title = "*" + title;
		SetTitle(title);
	}else
		SetTitle(AppName);
	msg_db_l(1);
}


void Tsunami::OnUpdate(Observable *o)
{
	if (o->GetName() == "AudioOutput"){
		ForceRedraw();
		UpdateMenu();
		float peak_r, peak_l;
		output->GetPeaks(peak_r, peak_l);
		peak_meter->Set(peak_r, peak_l);
	}else // "Data" or "AudioView"
		UpdateMenu();
}


void Tsunami::OnExit()
{
	if (AllowTermination())
		delete(this);
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
	if (!cur_audio->used)
		return;
	if (cur_audio->filename == "")
		OnSaveAs();
	else
		storage->Save(cur_audio, cur_audio->filename);
}


void Tsunami::OnSaveAs()
{
	if (!cur_audio->used)
		return;
	if (storage->AskSave(this))
		storage->Save(cur_audio, HuiFilename);
}

void Tsunami::OnExport()
{
	if (!cur_audio->used)
		return;
	if (storage->AskSaveExport(this))
		storage->Export(cur_audio, HuiFilename);
}
