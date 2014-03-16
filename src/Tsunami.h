/*
 * Tsunami.h
 *
 *  Created on: 21.03.2012
 *      Author: michi
 */

#ifndef TSUNAMI_H_
#define TSUNAMI_H_

#include "Stuff/Observer.h"
#include "lib/hui/hui.h"

class Observer;
class HuiWindow;
class AudioFile;
class AudioView;
class PluginManager;
class Slider;
class Log;
class AudioInput;
class AudioOutput;
class AudioRenderer;
class Storage;
class Progress;
class PeakMeter;
class Clipboard;
class SampleManager;
class MixingConsole;

class Tsunami : public Observer, public HuiWindow
{
public:
	Tsunami(Array<string> arg);
	virtual ~Tsunami();

	bool HandleArguments(Array<string> arg);
	void LoadKeyCodes();
	int Run();

	void OnAbout();
	void OnSendBugReport();

	void OnUpdate(Observable *o);
	void OnCommand(const string &id);

	void OnEvent();

	void OnNew();
	void OnOpen();
	void OnSave();
	void OnSaveAs();


	void OnCopy();
	void OnPaste();
	void OnDelete();
	void OnExport();
	void OnUndo();
	void OnRedo();
	void OnAddTrack();
	void OnAddTimeTrack();
	void OnAddMidiTrack();
	void OnDeleteTrack();
	void OnSubFromSelection();
	void OnInsertAdded();
	void OnRemoveAdded();
	void OnTrackImport();
	void OnAddLevel();
	void OnDeleteLevel();
	void OnSampleManager();
	void OnMixingConsole();
	void OnSubImport();
	void OnAudioProperties();
	void OnTrackProperties();
	void OnSubProperties();
	void OnSettings();
	void OnCloseFile();
	void OnPlay();
	void OnPlayLoop();
	void OnPause();
	void OnStop();
	void OnVolume();
	void OnRecord();
	void OnCurLevel();
	void OnCurLevelUp();
	void OnCurLevelDown();
	void OnViewPeaksMax();
	void OnViewPeaksMean();
	void OnZoomIn();
	void OnZoomOut();
	void OnViewMono();
	void OnViewStereo();
	void OnViewOptimal();
	void OnSelectNone();
	void OnSelectAll();
	void OnShowLog();
	void OnFindAndExecutePlugin();
	void OnExit();

	//bool FileDialog(int kind, bool save, bool force_in_root_dir);
	bool AllowTermination();
	bool Save();

	void UpdateMenu();

	AudioFile *audio;

	AudioView *view;

	PeakMeter *peak_meter;
	Slider *volume_slider;

	Storage *storage;

	Progress *progress;
	Log *log;

	AudioOutput *output;
	AudioInput *input;
	AudioRenderer *renderer;

	PluginManager *plugin_manager;
	Clipboard *clipboard;

	SampleManager *sample_manager;
	MixingConsole *mixing_console;
};

extern Tsunami *tsunami;

#endif /* TSUNAMI_H_ */
