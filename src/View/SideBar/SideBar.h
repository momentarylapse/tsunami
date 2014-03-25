/*
 * SideBar.h
 *
 *  Created on: 25.03.2014
 *      Author: michi
 */

#ifndef SIDEBAR_H_
#define SIDEBAR_H_

#include "../../lib/hui/hui.h"
#include "../../Stuff/Observable.h"

class AudioFile;
class Track;
class TrackDialog;
class AudioFileDialog;
class AudioView;

class SideBarConsole : public HuiPanel
{
public:
	SideBarConsole(const string &_title)
	{ title = _title; }
	string title;
};

class SideBar : public HuiPanel, public Observable
{
public:
	SideBar(AudioView *view, AudioFile *audio);
	virtual ~SideBar();

	void OnClose();
	virtual void OnShow();
	virtual void OnHide();

	enum
	{
		AUDIO_FILE_DIALOG,
		TRACK_DIALOG,
		NUM_CONSOLES
	};

	void SetTrack(Track *t);
	void Choose(int console);
	void Open(int console);
	bool IsActive(int console);
	int active_console;
	bool visible;

	AudioFileDialog *audio_file_dialog;
	TrackDialog *track_dialog;
};

#endif /* BOTTOMBAR_H_ */
