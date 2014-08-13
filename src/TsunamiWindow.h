/*
 * TsunamiWindow.h
 *
 *  Created on: 21.03.2012
 *      Author: michi
 */

#ifndef TSUNAMIWINDOW_H_
#define TSUNAMIWINDOW_H_

#include "Stuff/Observer.h"
#include "lib/hui/hui.h"

class Observer;
class HuiWindow;
class AudioFile;
class AudioView;
class SideBar;
class BottomBar;

class TsunamiWindow : public Observer, public HuiWindow
{
public:
	TsunamiWindow();
	virtual ~TsunamiWindow();

	void OnAbout();
	void OnSendBugReport();

	void OnUpdate(Observable *o, const string &message);
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
	void OnFxConsole();
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

	AudioView *view;

	AudioFile *audio;

	SideBar *side_bar;
	BottomBar *bottom_bar;
};

#endif /* TSUNAMIWINDOW_H_ */
