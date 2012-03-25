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
#include "Data/AudioFile.h"
#include "View/AudioView.h"
#include "Storage/Storage.h"
#include "Stuff/Progress.h"

class Observer;
class CHuiWindow;
class AudioFile;
class AudioView;

class Tsunami : public Observer, public CHuiWindow
{
public:
	Tsunami(Array<string> arg);
	virtual ~Tsunami();

	bool HandleArguments(Array<string> arg);
	void LoadKeyCodes();
	int Run();

	void IdleFunction();

	void OnAbout();
	void OnSendBugReport();

	void OnUpdate(Observable *o);
	void OnCommand(const string &id);
	void OnDraw();

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
	void OnInsertAdded();
	void OnRemoveAdded();
	void OnTrackImport();
	void OnSubImport();
	void OnAudioProperties();
	void OnTrackProperties();
	void OnSubProperties();
	void OnSettings();
	void OnCloseFile();
	void OnPlay();
	void OnPlayLoop();
	void OnStop();
	void OnRecord();
	void OnShowLog();
	void OnFindAndExecutePlugin();
	void OnExit();

	enum{
		LOG_ERROR,
		LOG_WARNING,
		LOG_INFO
	};

	void Log(int type, const string &message);

	//bool FileDialog(int kind, bool save, bool force_in_root_dir);
	bool AllowTermination(AudioFile *a = NULL);

	//void Draw();
	void ForceRedraw();
	bool force_redraw;
	void UpdateMenu();

	AudioFile *audio[2];
	AudioFile *cur_audio;

	AudioView *view;

	Storage *storage;

	Progress *progress;
};

extern Tsunami *tsunami;

#endif /* TSUNAMI_H_ */
