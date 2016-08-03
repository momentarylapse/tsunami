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

class TsunamiWindow : public HuiWindow
{
public:
	TsunamiWindow();
	virtual ~TsunamiWindow();

	void onAbout();
	void onSendBugReport();

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
	void onScaleSample();
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
	void onSelectExpand();
	void onShowLog();
	void onFindAndExecutePlugin();
	void onMenuExecuteEffect();
	void onMenuExecuteMidiEffect();
	void onMenuExecuteSongPlugin();
	void onMenuExecuteTsunamiPlugin();
	void onExit();

	//bool FileDialog(int kind, bool save, bool force_in_root_dir);
	bool allowTermination();
	bool save();

	void updateMenu();

	Observer *observer;
	void onUpdate(Observable *o, const string &message);

	AudioView *view;

	Song *song;

	SideBar *side_bar;
	BottomBar *bottom_bar;
	MiniBar *mini_bar;

	Array<TsunamiPlugin*> plugins;
};

#endif /* TSUNAMIWINDOW_H_ */
