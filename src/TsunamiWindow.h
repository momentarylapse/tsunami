/*
 * TsunamiWindow.h
 *
 *  Created on: 21.03.2012
 *      Author: michi
 */

#ifndef TSUNAMIWINDOW_H_
#define TSUNAMIWINDOW_H_

#include "lib/hui/hui.h"

class Song;
class AudioView;
class SideBar;
class BottomBar;
class MiniBar;
class Session;
class Tsunami;

class TsunamiWindow : public hui::Window
{
public:
	TsunamiWindow(Session *session);
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
	void onTrackRender();
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
	void onInsertTimeInterval();
	void onDeleteTimeInterval();
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
	void onMenuExecuteEffect();
	void onMenuExecuteMidiEffect();
	void onMenuExecuteSongPlugin();
	void onMenuExecuteTsunamiPlugin();
	void onExit();

	//bool FileDialog(int kind, bool save, bool force_in_root_dir);
	bool allowTermination();
	bool save();

	void updateMenu();

	void onSideBarUpdate();
	void onBottomBarUpdate();
	void onUpdate();

	AudioView *view;

	Array<string> menu_layer_names;

	Song *song;

	Session *session;

	SideBar *side_bar;
	BottomBar *bottom_bar;
	MiniBar *mini_bar;

	bool auto_delete;

	Tsunami *app;
};

#endif /* TSUNAMIWINDOW_H_ */
