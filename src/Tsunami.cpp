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
#include "View/Dialog/MixingConsole.h"
#include "View/Dialog/FxConsole.h"
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


	int width = HuiConfig.getInt("Window.Width", 800);
	int height = HuiConfig.getInt("Window.Height", 600);
	bool maximized = HuiConfig.getBool("Window.Maximized", true);

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
	HuiAddCommandM("show_mixing_console", "", -1, this, &Tsunami::OnMixingConsole);
	HuiAddCommandM("show_fx_console", "", -1, this, &Tsunami::OnFxConsole);
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
	HuiAddCommandM("view_peaks_max", "", -1, this, &Tsunami::OnViewPeaksMax);
	HuiAddCommandM("view_peaks_mean", "", -1, this, &Tsunami::OnViewPeaksMean);
	HuiAddCommandM("view_optimal", "", -1, this, &Tsunami::OnViewOptimal);
	HuiAddCommandM("zoom_in", "", -1, this, &Tsunami::OnZoomIn);
	HuiAddCommandM("zoom_out", "", -1, this, &Tsunami::OnZoomOut);


	// table structure
	SetSize(width, height);
	SetBorderWidth(0);
	AddControlTable("", 0, 0, 1, 3, "root_table");
	SetTarget("root_table", 0);
	AddControlTable("", 0, 0, 4, 1, "main_table");
	SetBorderWidth(5);
	AddControlTable("!noexpandy", 0, 3, 3, 1, "bottom_table");

	// bottom
	SetTarget("bottom_table", 0);
	AddControlTable("!noexpandy", 0, 0, 7, 1, "output_table");
	AddControlTable("!noexpandy", 1, 0, 7, 1, "edit_midi_table");
	HideControl("edit_midi_table", true);

	// main table
	SetBorderWidth(0);
	SetTarget("main_table", 0);
	AddDrawingArea("!grabfocus", 0, 0, 0, 0, "area");

	// output table
	SetBorderWidth(5);

	// edit midi table
	SetTarget("edit_midi_table", 0);
	AddSeparator("", 0, 0, 0, 0, "");
	AddText("Pitch", 1, 0, 0, 0, "");
	AddSpinButton("60\\0\\90", 2, 0, 0, 0, "pitch_offset");
	AddText("Unterteilung", 3, 0, 0, 0, "");
	AddSpinButton("4\\1", 4, 0, 0, 0, "beat_partition");
	AddButton("Beenden", 5, 0, 0, 0, "close_edit_midi_mode");


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


	sample_manager = new SampleManager(audio, this, true);

	mixing_console = new MixingConsole(audio, output);
	Embed(mixing_console, "main_table", 0, 1);
	mixing_console->Hide();

	// create (link) PluginManager after all other components are ready
	plugin_manager = new PluginManager;
	plugin_manager->AddPluginsToMenu();
	plugin_manager->LinkAppScriptData();

	Subscribe(view);
	Subscribe(audio);
	Subscribe(output, "StateChange");
	Subscribe(clipboard);
	Subscribe(mixing_console);
	Subscribe(view->fx_console);

	UpdateMenu();

	log->Info(AppName + " " + AppVersion);
	log->Info(_("  viel Erfolg!"));

	audio->NewWithOneTrack(Track::TYPE_AUDIO, DEFAULT_SAMPLE_RATE);

	HandleArguments(arg);

	Show();

	HuiRunLaterM(0.01f, this, &Tsunami::OnViewOptimal);
}

Tsunami::~Tsunami()
{
	Unsubscribe(view);
	Unsubscribe(audio);
	Unsubscribe(output);
	Unsubscribe(clipboard);
	Unsubscribe(mixing_console);
	Unsubscribe(view->fx_console);

	int w, h;
	GetSizeDesired(w, h);
	HuiConfig.setInt("Window.Width", w);
	HuiConfig.setInt("Window.Height", h);
	HuiConfig.setBool("Window.Maximized", IsMaximized());

	delete(mixing_console);
	delete(sample_manager);
	delete(plugin_manager);
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
	audio->action_manager->BeginActionGroup();
	Track *t = audio->AddTrack(Track::TYPE_TIME);
	if (t)
		t->AddBars(-1, 90, 4, 10);
	audio->action_manager->EndActionGroup();
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
	plugin_manager->FindAndExecutePlugin();
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

void Tsunami::OnMixingConsole()
{
	view->fx_console->Show(false);
	mixing_console->Show(!mixing_console->enabled);
}

void Tsunami::OnFxConsole()
{
	mixing_console->Show(false);
	view->fx_console->Show(!view->fx_console->enabled);
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
	renderer->loop_if_allowed = !renderer->loop_if_allowed;
	UpdateMenu();
}

void Tsunami::OnPlay()
{
	renderer->Prepare(audio, audio->GetPlaybackSelection(), true);
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
	Check("play_loop", renderer->loop_if_allowed);
	// view
	Check("show_mixing_console", mixing_console->enabled);
	Check("show_fx_console", view->fx_console->enabled);

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


void Tsunami::OnUpdate(Observable *o, const string &message)
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
