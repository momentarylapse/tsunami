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
class CHuiWindow;
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

class Tsunami : public Observer, public CHuiWindow
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
	void OnDeleteTrack();
	void OnSubFromSelection();
	void OnInsertAdded();
	void OnRemoveAdded();
	void OnTrackImport();
	void OnAddLevel();
	void OnDeleteLevel();
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
	void OnViewGridTime();
	void OnViewGridBars();
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

	//void Draw();
	void ForceRedraw();
	bool force_redraw;
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

	PluginManager *plugins;
	Clipboard *clipboard;
};

extern Tsunami *tsunami;

#endif /* TSUNAMI_H_ */
