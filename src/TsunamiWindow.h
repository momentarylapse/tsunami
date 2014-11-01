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
	void onExport();
	void onUndo();
	void onRedo();
	void onAddTrack();
	void onAddTimeTrack();
	void onAddMidiTrack();
	void onDeleteTrack();
	void onTrackEditMidi();
	void onTrackEditFX();
	void onSubFromSelection();
	void onInsertAdded();
	void onRemoveAdded();
	void onTrackImport();
	void onAddLevel();
	void onDeleteLevel();
	void onSampleManager();
	void onMixingConsole();
	void onFxConsole();
	void onSubImport();
	void onAudioProperties();
	void onTrackProperties();
	void onSubProperties();
	void onSettings();
	void onPlay();
	void onPlayLoop();
	void onPause();
	void onStop();
	void onRecord();
	void onCurLevel();
	void onCurLevelUp();
	void onCurLevelDown();
	void onViewPeaksMax();
	void onViewPeaksMean();
	void onZoomIn();
	void onZoomOut();
	void onViewMono();
	void onViewStereo();
	void onViewOptimal();
	void onSelectNone();
	void onSelectAll();
	void onShowLog();
	void onFindAndExecutePlugin();
	void onExit();

	//bool FileDialog(int kind, bool save, bool force_in_root_dir);
	bool allowTermination();
	bool save();

	void updateMenu();

	AudioView *view;

	AudioFile *audio;

	SideBar *side_bar;
	BottomBar *bottom_bar;
};

#endif /* TSUNAMIWINDOW_H_ */
