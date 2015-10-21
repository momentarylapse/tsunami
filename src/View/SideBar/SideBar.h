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
class FxConsole;
class SynthConsole;
class MidiFxConsole;
class BarsConsole;
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
		BARS_CONSOLE,
		SAMPLE_CONSOLE,
		GLOBAL_FX_CONSOLE,
		TRACK_CONSOLE,
		MIDI_EDITOR,
		FX_CONSOLE,
		SYNTH_CONSOLE,
		MIDI_FX_CONCOLE,
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
	BarsConsole *bars_console;
	FxConsole *global_fx_console;
	TrackConsole *track_console;
	MidiEditor *midi_editor;
	FxConsole *fx_console;
	SynthConsole *synth_console;
	MidiFxConsole *midi_fx_console;
	SampleRefDialog *sample_ref_dialog;
	SampleManager *sample_manager;

	Array<SideBarConsole*> consoles;
	void addConsole(SideBarConsole *c);
};

#endif /* BOTTOMBAR_H_ */
