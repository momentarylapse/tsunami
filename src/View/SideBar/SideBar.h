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

class Song;
class SongConsole;
class LevelConsole;
class TrackConsole;
class MidiEditor;
class SampleRefDialog;
class SampleManager;
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
	SideBar(AudioView *view, Song *song);
	virtual ~SideBar();

	void onClose();
	void onChoose();

	void _show();
	void _hide();

	enum
	{
		SONG_CONSOLE,
		LEVEL_CONSOLE,
		SAMPLE_CONSOLE,
		TRACK_CONSOLE,
		TRACK_MIDI_EDITOR,
		SAMPLEREF_DIALOG,
		NUM_CONSOLES
	};

	void choose(int console);
	void open(int console);
	bool isActive(int console);
	int active_console;
	bool visible;

	SongConsole *song_console;
	LevelConsole *level_console;
	TrackConsole *track_console;
	MidiEditor *track_midi_editor;
	SampleRefDialog *sample_ref_dialog;
	SampleManager *sample_manager;

	Array<SideBarConsole*> consoles;
	void addConsole(SideBarConsole *c);
};

#endif /* BOTTOMBAR_H_ */
