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
class Song;
class AudioView;
class SideBar;
class BottomBar;
class MiniBar;
class TsunamiPlugin;

class TsunamiWindow : public Observer, public HuiWindow
{
public:
	TsunamiWindow();
	virtual ~TsunamiWindow();

	void onAbout();
	void onSendBugReport();

	void onUpdate(Observable *o, const string &message);
	void onCommand(const string &id);

	void onEvent();

	void onNew();
	void onOpen();
	void onSave();
	void onSaveAs();


	void onCopy();
	void onPaste();
	void onDelete();
	void onEditMulti();
	void onExport();
	void onUndo();
	void onRedo();
	void onAddTrack();
	void onAddTimeTrack();
	void onAddMidiTrack();
	void onDeleteTrack();
	void onTrackEditMidi();
	void onTrackEditFX();
	void onTrackAddMarker();
	void onSampleFromSelection();
	void onInsertSample();
	void onRemoveSample();
	void onTrackImport();
	void onAddLevel();
	void onDeleteLevel();
	void onSampleManager();
	void onMixingConsole();
	void onFxConsole();
	void onSampleImport();
	void onSongProperties();
	void onTrackProperties();
	void onSampleProperties();
	void onEditMarker();
	void onDeleteMarker();
	void onSettings();
	void onPlay();
	void onPlayLoop();
	void onPause();
	void onStop();
	void onRecord();
	void onCurLevel();
	void onCurLevelUp();
	void onCurLevelDown();
	void onLevelManager();
	void onBarsManager();
	void onViewPeaksMax();
	void onViewPeaksMean();
	void onViewPeaksBoth();
	void onViewMidiDefault();
	void onViewMidiTab();
	void onViewMidiScore();
	void onZoomIn();
	void onZoomOut();
	void onViewMono();
	void onViewStereo();
	void onViewOptimal();
	void onSelectNone();
	void onSelectAll();
	void onShowLog();
	void onFindAndExecutePlugin();
	void onMenuExecutePlugin();
	void onExit();

	//bool FileDialog(int kind, bool save, bool force_in_root_dir);
	bool allowTermination();
	bool save();

	void updateMenu();

	AudioView *view;

	Song *song;

	SideBar *side_bar;
	BottomBar *bottom_bar;
	MiniBar *mini_bar;

	Array<TsunamiPlugin*> active_plugins;
};

#endif /* TSUNAMIWINDOW_H_ */
