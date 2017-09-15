/*
 * TsunamiWindow.h
 *
 *  Created on: 21.03.2012
 *      Author: michi
 */

#ifndef TSUNAMIWINDOW_H_
#define TSUNAMIWINDOW_H_

#include "lib/hui/hui.h"

class HuiWindow;
class Song;
class AudioView;
class SideBar;
class BottomBar;
class MiniBar;
class TsunamiPlugin;
class Tsunami;
class Observable;

class TsunamiWindow : public hui::Window
{
public:
	TsunamiWindow(Tsunami *tsunami);
	virtual ~TsunamiWindow();
	virtual void onDestroy();

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
	void onPasteAsSamples();
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
	void onTrackAddMarker();
	void onSampleFromSelection();
	void onInsertSample();
	void onRemoveSample();
	void onTrackImport();
	void onAddLayer();
	void onDeleteLayer();
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
	void onCurLayer();
	void onCurLayerUp();
	void onCurLayerDown();
	void onLayerManager();
	void onAddBars();
	void onAddPause();
	void onDeleteBars();
	void onEditBars();
	void onScaleBars();
	void onBarsModifyMidi();
	void onViewMidiDefault();
	void onViewMidiTab();
	void onViewMidiScore();
	void onZoomIn();
	void onZoomOut();
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

	void onUpdate(Observable *o);

	AudioView *view;

	Song *song;

	SideBar *side_bar;
	BottomBar *bottom_bar;
	MiniBar *mini_bar;

	Array<TsunamiPlugin*> plugins;
	bool die_on_plugin_stop;
	bool auto_delete;

	Tsunami *app;
};

#endif /* TSUNAMIWINDOW_H_ */
