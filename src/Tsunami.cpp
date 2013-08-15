/*
 * Tsunami.cpp
 *
 *  Created on: 21.03.2012
 *      Author: michi
 */

#include "lib/hui/hui.h"
#include "Tsunami.h"
#include "Data/AudioFile.h"
#include "View/Dialog/NewDialog.h"
#include "View/Dialog/CaptureDialog.h"
#include "View/Dialog/SettingsDialog.h"
#include "View/Dialog/SampleManager.h"
#include "View/Dialog/MidiPatternManager.h"
#include "View/Helper/Slider.h"
#include "View/Helper/Progress.h"
#include "View/Helper/PeakMeter.h"
#include "View/AudioView.h"
#include "Plugins/PluginManager.h"
#include "Storage/Storage.h"
#include "Stuff/Log.h"
#include "Stuff/Clipboard.h"
#include "Audio/AudioOutput.h"
#include "Audio/AudioInput.h"
#include "Audio/AudioRenderer.h"

#include "Plugins/FastFourierTransform.h"

Tsunami *tsunami = NULL;
extern string AppName;
extern string AppVersion;

HuiTimer debug_timer;

Tsunami::Tsunami(Array<string> arg) :
	HuiWindow(AppName, -1, -1, 800, 600, NULL, false, HuiWinModeResizable | HuiWinModeControls)
{
	tsunami = this;

	progress = new Progress;
	log = new Log(this);

	clipboard = new Clipboard;

	output = new AudioOutput;
	input = new AudioInput;
	renderer = new AudioRenderer;


	int width = HuiConfigReadInt("Window.Width", 800);
	int height = HuiConfigReadInt("Window.Height", 600);
	bool maximized = HuiConfigReadBool("Window.Maximized", true);

	//HuiAddKeyCode("insert_added", KEY_RETURN);
	//HuiAddKeyCode("remove_added", KEY_BACKSPACE);

	HuiAddCommandM("new", "hui:new", KEY_N + KEY_CONTROL, this, &Tsunami::OnNew);
	HuiAddCommandM("open", "hui:open", KEY_O + KEY_CONTROL, this, &Tsunami::OnOpen);
	HuiAddCommandM("save", "hui:save", KEY_S + KEY_CONTROL, this, &Tsunami::OnSave);
	HuiAddCommandM("save_as", "hui:save-as", KEY_S + KEY_CONTROL + KEY_SHIFT, this, &Tsunami::OnSaveAs);
	HuiAddCommandM("copy", "hui:copy", KEY_C + KEY_CONTROL, this, &Tsunami::OnCopy);
	HuiAddCommandM("paste", "hui:paste", KEY_V + KEY_CONTROL, this, &Tsunami::OnPaste);
	HuiAddCommandM("delete", "hui:delete", -1, this, &Tsunami::OnDelete);
	HuiAddCommandM("export_selection", "", KEY_X + KEY_CONTROL, this, &Tsunami::OnExport);
	HuiAddCommandM("undo", "hui:undo", KEY_Z + KEY_CONTROL, this, &Tsunami::OnUndo);
	HuiAddCommandM("redo", "hui:redo", KEY_Y + KEY_CONTROL, this, &Tsunami::OnRedo);
	HuiAddCommandM("add_track", "hui:add", -1, this, &Tsunami::OnAddTrack);
	HuiAddCommandM("add_time_track", "hui:add", -1, this, &Tsunami::OnAddTimeTrack);
	HuiAddCommandM("add_midi_track", "hui:add", -1, this, &Tsunami::OnAddMidiTrack);
	HuiAddCommandM("delete_track", "hui:delete", -1, this, &Tsunami::OnDeleteTrack);
	HuiAddCommandM("level_add", "hui:add", -1, this, &Tsunami::OnAddLevel);
	HuiAddCommandM("level_delete", "hui:delete", -1, this, &Tsunami::OnDeleteLevel);
	HuiAddCommandM("level_up", "hui:up", -1, this, &Tsunami::OnCurLevelUp);
	HuiAddCommandM("level_down", "hui:down", -1, this, &Tsunami::OnCurLevelDown);
	HuiAddCommandM("sample_manager", "", -1, this, &Tsunami::OnSampleManager);
	HuiAddCommandM("midi_pattern_manager", "", -1, this, &Tsunami::OnMidiPatternManager);
	HuiAddCommandM("sub_from_selection", "hui:cut", -1, this, &Tsunami::OnSubFromSelection);
	HuiAddCommandM("insert_added", "", KEY_I + KEY_CONTROL, this, &Tsunami::OnInsertAdded);
	HuiAddCommandM("remove_added", "", -1, this, &Tsunami::OnRemoveAdded);
	HuiAddCommandM("track_import", "", -1, this, &Tsunami::OnTrackImport);
	HuiAddCommandM("sub_import", "", -1, this, &Tsunami::OnSubImport);
	HuiAddCommandM("wave_properties", "", KEY_F4, this, &Tsunami::OnAudioProperties);
	HuiAddCommandM("track_properties", "", -1, this, &Tsunami::OnTrackProperties);
	HuiAddCommandM("sub_properties", "", -1, this, &Tsunami::OnSubProperties);
	HuiAddCommandM("settings", "", -1, this, &Tsunami::OnSettings);
	HuiAddCommandM("close_file", "hui:close", KEY_W + KEY_CONTROL, this, &Tsunami::OnCloseFile);
	HuiAddCommandM("play", "hui:media-play", -1, this, &Tsunami::OnPlay);
	HuiAddCommandM("play_loop", "", -1, this, &Tsunami::OnPlayLoop);
	HuiAddCommandM("pause", "hui:media-pause", -1, this, &Tsunami::OnPause);
	HuiAddCommandM("stop", "hui:media-stop", -1, this, &Tsunami::OnStop);
	HuiAddCommandM("record", "hui:media-record", -1, this, &Tsunami::OnRecord);
	HuiAddCommandM("show_log", "", -1, this, &Tsunami::OnShowLog);
	HuiAddCommandM("about", "", -1, this, &Tsunami::OnAbout);
	HuiAddCommandM("run_plugin", "hui:execute", KEY_RETURN + KEY_SHIFT, this, &Tsunami::OnFindAndExecutePlugin);
	HuiAddCommandM("exit", "hui:quit", KEY_Q + KEY_CONTROL, this, &Tsunami::OnExit);

	HuiAddCommandM("select_all", "", KEY_A + KEY_CONTROL, this, &Tsunami::OnSelectAll);
	HuiAddCommandM("select_nothing", "", -1, this, &Tsunami::OnSelectNone);
	HuiAddCommandM("view_mono", "", -1, this, &Tsunami::OnViewMono);
	HuiAddCommandM("view_stereo", "", -1, this, &Tsunami::OnViewStereo);
	HuiAddCommandM("view_grid_time", "", -1, this, &Tsunami::OnViewGridTime);
	HuiAddCommandM("view_grid_bars", "", -1, this, &Tsunami::OnViewGridBars);
	HuiAddCommandM("view_peaks_max", "", -1, this, &Tsunami::OnViewPeaksMax);
	HuiAddCommandM("view_peaks_mean", "", -1, this, &Tsunami::OnViewPeaksMean);
	HuiAddCommandM("view_optimal", "", -1, this, &Tsunami::OnViewOptimal);
	HuiAddCommandM("zoom_in", "", -1, this, &Tsunami::OnZoomIn);
	HuiAddCommandM("zoom_out", "", -1, this, &Tsunami::OnZoomOut);


	// create the window
	SetSize(width, height);
	SetBorderWidth(0);
	AddControlTable("", 0, 0, 1, 2, "root_table");
	SetTarget("root_table", 0);
	AddControlTable("", 0, 0, 3, 1, "main_table");
	SetBorderWidth(5);
	AddControlTable("!noexpandy", 0, 1, 7, 1, "output_table");
	SetBorderWidth(0);
	SetTarget("main_table", 0);
	AddControlTable("!noexpandx", 1, 0, 1, 1, "track_dialog_table");
	HideControl("track_dialog_table", true);
	AddControlTable("!noexpandx", 2, 0, 1, 1, "audio_dialog_table");
	HideControl("audio_dialog_table", true);
	SetBorderWidth(5);
	SetTarget("output_table", 0);
	AddButton("", 0, 0, 0, 0, "play");
	AddButton("", 1, 0, 0, 0, "pause");
	AddButton("", 2, 0, 0, 0, "stop");
	AddDrawingArea("!width=100", 3, 0, 0, 0, "output_peaks");
	peak_meter = new PeakMeter(this, "output_peaks", output);
	AddSlider("!width=100", 4, 0, 0, 0, "output_volume_slider");
	AddSpinButton("!width=50\\0\\0\\100", 5, 0, 0, 0, "output_volume");
	AddText("!width=20\\%", 6, 0, 0, 0, "output_label_percent");
	volume_slider = new Slider(this, "output_volume_slider", "output_volume", 0, 1, 100, (void(HuiEventHandler::*)())&Tsunami::OnVolume, output->GetVolume());
	AllowEvents("key");
	toolbar[0]->SetByID("toolbar");
	//ToolbarConfigure(false, true);

	SetMenu(HuiCreateResourceMenu("menu"));
	//ToolBarConfigure(true, true);
	SetMaximized(maximized);

	// events
	EventM("hui:close", this, &Tsunami::OnExit);
	for (int i=0;i<256;i++)
		EventM(format("jump_to_level_%d", i), this, &Tsunami::OnCurLevel);


	audio = new AudioFile;

	storage = new Storage;

	view = new AudioView(this, audio);

	plugins = new PluginManager;
	plugins->AddPluginsToMenu();
	plugins->LinkAppScriptData();


	sample_manager = new SampleManager(audio, this, true);
	midi_pattern_manager = new MidiPatternManager(audio, this, true);

	Subscribe(view);
	Subscribe(audio);
	Subscribe(output, "StateChange");
	Subscribe(clipboard);

	UpdateMenu();

	log->Info(AppName + " " + AppVersion);
	log->Info(_("  viel Erfolg!"));

	audio->NewWithOneTrack(Track::TYPE_AUDIO, DEFAULT_SAMPLE_RATE);

	HandleArguments(arg);

	Show();
}

Tsunami::~Tsunami()
{
	Unsubscribe(view);
	Unsubscribe(audio);
	Unsubscribe(output);
	Unsubscribe(clipboard);

	irect r = GetOuteriorDesired();
	HuiConfigWriteInt("Window.Width", r.x2 - r.x1);
	HuiConfigWriteInt("Window.Height", r.y2 - r.y1);
	HuiConfigWriteInt("Window.X", r.x1);
	HuiConfigWriteInt("Window.Y", r.y1);
	HuiConfigWriteBool("Window.Maximized", IsMaximized());

	delete(sample_manager);
	delete(plugins);
	delete(storage);
	delete(view);
	delete(output);
	delete(input);
	delete(audio);
	delete(renderer);
	HuiEnd();
}


int Tsunami::Run()
{
	return HuiRun();
}



void Tsunami::OnAbout()
{
	HuiAboutBox(this);
}



void Tsunami::OnAddTrack()
{
	audio->AddTrack(Track::TYPE_AUDIO);
}

void Tsunami::OnAddTimeTrack()
{
	audio->AddTrack(Track::TYPE_TIME);
}

void Tsunami::OnAddMidiTrack()
{
	audio->AddTrack(Track::TYPE_MIDI);
}

void Tsunami::OnDeleteTrack()
{
	if (audio->used){
		if (audio->track.num < 2){
			log->Error(_("Es muss mindestens eine Spur existieren"));
			return;
		}
		audio->DeleteTrack(get_track_index(view->cur_track));
	}
}

void Tsunami::OnCloseFile()
{
	audio->Reset();
}

void Tsunami::LoadKeyCodes()
{
}

void Tsunami::OnAudioProperties()
{
	view->ExecuteAudioDialog(this);
}

void Tsunami::OnTrackProperties()
{
	view->ExecuteTrackDialog(this);
}

void Tsunami::OnSubProperties()
{
	view->ExecuteSubDialog(this);
}

void Tsunami::OnShowLog()
{
	log->Show();
}

void Tsunami::OnUndo()
{
	audio->action_manager->Undo();
}

void Tsunami::OnRedo()
{
	audio->action_manager->Redo();
}

void Tsunami::OnSendBugReport()
{
}


string title_filename(const string &filename)
{
	if (filename.num > 0)
		return filename.basename();// + " (" + filename.dirname() + ")";
	return _("Unbenannt");
}

bool Tsunami::AllowTermination()
{
	if (!audio->action_manager->IsSave()){
		string answer = HuiQuestionBox(this, _("Frage"), format(_("'%s'\nDatei speichern?"), title_filename(audio->filename).c_str()), true);
		if (answer == "hui:yes"){
			/*if (!OnSave())
				return false;*/
			OnSave();
		}else if (answer == "hui:cancel")
			return false;
	}
	return true;
}

void Tsunami::OnCopy()
{
	if (audio->used)
		clipboard->Copy(audio);
}

void Tsunami::OnPaste()
{
	clipboard->Paste(audio);
}

void Tsunami::OnFindAndExecutePlugin()
{
	plugins->FindAndExecutePlugin();
}

void Tsunami::OnDelete()
{
	if (audio->used)
		audio->DeleteSelection(view->cur_level, false);
}

void Tsunami::OnSampleManager()
{
	sample_manager->Show();
}

void Tsunami::OnMidiPatternManager()
{
	midi_pattern_manager->Show();
}

void Tsunami::OnSubImport()
{
}

void Tsunami::OnCommand(const string & id)
{
}

void Tsunami::OnSettings()
{
	SettingsDialog *dlg = new SettingsDialog(this, false);
	dlg->Run();
}

void Tsunami::OnTrackImport()
{
	if (!audio->used)
		return;
	if (storage->AskOpenImport(this)){
		Track *t = audio->AddTrack(Track::TYPE_AUDIO);
		storage->LoadTrack(t, HuiFilename, audio->selection.start(), view->cur_level);
	}
}

bool Tsunami::HandleArguments(Array<string> arg)
{
	if (arg.num > 1)
		return storage->Load(audio, arg[1]);
	return false;
}

void Tsunami::OnRemoveAdded()
{
}

void Tsunami::OnPlayLoop()
{
	renderer->loop = !renderer->loop;
	UpdateMenu();
}

void Tsunami::OnPlay()
{
	renderer->Prepare(audio, audio->selection, false);
	output->Play(renderer);
}

void Tsunami::OnPause()
{
	output->Pause();
}

void Tsunami::OnStop()
{
	output->Stop();
}

void Tsunami::OnVolume()
{
	output->SetVolume(volume_slider->Get());
}

void Tsunami::OnInsertAdded()
{
	if (audio->used)
		audio->InsertSelectedSubs(view->cur_level);
}

void Tsunami::OnRecord()
{
	CaptureDialog *dlg = new CaptureDialog(this, false, audio);
	dlg->Run();
}

void Tsunami::OnAddLevel()
{
	if (audio->used)
		audio->AddLevel();
}

void Tsunami::OnDeleteLevel()
{
}

void Tsunami::OnCurLevel()
{
	view->cur_level = HuiGetEvent()->id.substr(14, -1)._int();
	UpdateMenu();
	view->ForceRedraw();
}

void Tsunami::OnCurLevelUp()
{
	if (view->cur_level < audio->level_name.num - 1){
		view->cur_level ++;
		UpdateMenu();
		view->ForceRedraw();
	}
}

void Tsunami::OnCurLevelDown()
{
	if (view->cur_level > 0){
		view->cur_level --;
		UpdateMenu();
		view->ForceRedraw();
	}
}

void Tsunami::OnSubFromSelection()
{
	if (audio->used)
		audio->CreateSubsFromSelection(view->cur_level);
}

void Tsunami::OnViewOptimal()
{
	view->OptimizeView();
}

void Tsunami::OnSelectNone()
{
	view->SelectNone();
}

void Tsunami::OnSelectAll()
{
	view->SelectAll();
}

void Tsunami::OnViewPeaksMax()
{
	view->SetPeaksMode(BufferBox::PEAK_MODE_MAXIMUM);
}

void Tsunami::OnViewPeaksMean()
{
	view->SetPeaksMode(BufferBox::PEAK_MODE_SQUAREMEAN);
}

void Tsunami::OnViewMono()
{
	view->SetShowMono(true);
}

void Tsunami::OnViewStereo()
{
	view->SetShowMono(false);
}

void Tsunami::OnViewGridBars()
{
	view->SetGridMode(view->GRID_MODE_BARS);
}

void Tsunami::OnViewGridTime()
{
	view->SetGridMode(view->GRID_MODE_TIME);
}

void Tsunami::OnZoomIn()
{
	view->ZoomIn();
}

void Tsunami::OnZoomOut()
{
	view->ZoomOut();
}

void Tsunami::UpdateMenu()
{
	msg_db_f("UpdateMenu", 1);
	bool selected = !audio->selection.empty();
// menu / toolbar
	// edit
	Enable("select_all", audio->used);
	Enable("select_nothing", audio->used);
	Enable("undo", audio->action_manager->Undoable());
	Enable("redo", audio->action_manager->Redoable());
	Enable("copy", selected || (audio->GetNumSelectedSubs() > 0));
	Enable("paste", clipboard->HasData());
	Enable("delete", selected || (audio->GetNumSelectedSubs() > 0));
	// file
	Enable("save", audio->used);
	Enable("save", audio->used);
	Enable("save_as", audio->used);
	Enable("close_file", audio->used);
	Enable("export_selection", audio->used);
	Enable("wave_properties", audio->used);
	// track
	Enable("track_import", audio->used);
	Enable("add_track", audio->used);
	Enable("add_time_track", audio->used);
	Enable("delete_track", view->cur_track);
	Enable("track_properties", view->cur_track);
	// level
	Enable("level_add", audio->used);
	Enable("level_delete", audio->used && (audio->level_name.num > 1));
	Enable("level_up", audio->used && (view->cur_level < audio->level_name.num -1));
	Enable("level_down", audio->used && (view->cur_level > 0));
	// sub
	Enable("sub_import", view->cur_track);
	Enable("sub_from_selection", selected);
	Enable("insert_added", audio->GetNumSelectedSubs() > 0);
	Enable("remove_added", audio->GetNumSelectedSubs() > 0);
	Enable("sub_properties", view->cur_sample);
	// sound
	Enable("play", audio->used);
	Enable("stop", output->IsPlaying());
	Enable("pause", output->IsPlaying());
	Check("play_loop", renderer->loop);

	HuiMenu *m = GetMenu()->GetSubMenuByID("menu_level_target");
	if (m){
		m->Clear();
		foreachib(string &l, audio->level_name, i)
			m->AddItemCheckable(l, format("jump_to_level_%d", i));
		Check(format("jump_to_level_%d", view->cur_level), true);
	}

	if (audio->used){
		string title = title_filename(audio->filename) + " - " + AppName;
		if (!audio->action_manager->IsSave())
			title = "*" + title;
		SetTitle(title);
	}else
		SetTitle(AppName);
}


void Tsunami::OnUpdate(Observable *o)
{
	if (o->GetName() == "AudioOutput"){
		view->ForceRedraw();
		UpdateMenu();
	}else // "Clipboard", "AudioFile" or "AudioView"
		UpdateMenu();
}


void Tsunami::OnExit()
{
	if (AllowTermination())
		delete(this);
}


void Tsunami::OnNew()
{
	NewDialog *d = new NewDialog(tsunami, false, audio);
	d->Run();
}


void Tsunami::OnOpen()
{
	if (storage->AskOpen(this))
		storage->Load(audio, HuiFilename);
}


void Tsunami::OnSave()
{
	if (!audio->used)
		return;
	if (audio->filename == "")
		OnSaveAs();
	else
		storage->Save(audio, audio->filename);
}


void Tsunami::OnSaveAs()
{
	if (!audio->used)
		return;
	if (storage->AskSave(this))
		storage->Save(audio, HuiFilename);
}

void Tsunami::OnExport()
{
	if (!audio->used)
		return;
	if (storage->AskSaveExport(this))
		storage->Export(audio, HuiFilename);
}
