/*
 * SideBar.h
 *
 *  Created on: 25.03.2014
 *      Author: michi
 */

#ifndef SIDEBAR_H_
#define SIDEBAR_H_

#include "../../lib/hui/hui.h"
#include "../../Stuff/Observer.h"

class AudioFile;
class AudioFileConsole;
class TrackConsole;
class MidiEditor;
class SubDialog;
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

	void onClose();
	virtual void onShow();
	virtual void onHide();

	enum
	{
		AUDIOFILE_CONSOLE,
		TRACK_CONSOLE,
		TRACK_MIDI_EDITOR,
		SUB_DIALOG,
		NUM_CONSOLES
	};

	void choose(int console);
	void open(int console);
	bool isActive(int console);
	int active_console;
	bool visible;

	AudioFileConsole *audio_file_console;
	TrackConsole *track_console;
	MidiEditor *track_midi_editor;
	SubDialog *sub_dialog;
};

#endif /* BOTTOMBAR_H_ */
