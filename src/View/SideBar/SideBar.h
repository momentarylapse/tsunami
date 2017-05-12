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
class LayerConsole;
class TrackConsole;
class MidiEditorConsole;
class SampleRefConsole;
class SampleManagerConsole;
class FxConsole;
class CurveConsole;
class SynthConsole;
class MidiFxConsole;
class CaptureConsole;
class AudioView;

class SideBarConsole : public hui::Panel
{
public:
	SideBarConsole(const string &_title)
	{ title = _title; }
	string title;

	virtual void onEnter(){}
	virtual void onLeave(){}
};

class SideBar : public hui::Panel, public Observable
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
		LAYER_CONSOLE,
		SAMPLE_CONSOLE,
		GLOBAL_FX_CONSOLE,
		TRACK_CONSOLE,
		MIDI_EDITOR_CONSOLE,
		FX_CONSOLE,
		CURVE_CONSOLE,
		SYNTH_CONSOLE,
		MIDI_FX_CONCOLE,
		SAMPLEREF_CONSOLE,
		CAPTURE_CONSOLE,
		NUM_CONSOLES
	};

	void choose(int console);
	void open(int console);
	bool isActive(int console);
	int active_console;
	bool visible;

	SongConsole *song_console;
	LayerConsole *layer_console;
	FxConsole *global_fx_console;
	TrackConsole *track_console;
	MidiEditorConsole *midi_editor_console;
	FxConsole *fx_console;
	CurveConsole *curve_console;
	SynthConsole *synth_console;
	MidiFxConsole *midi_fx_console;
	SampleRefConsole *sample_ref_console;
	SampleManagerConsole *sample_manager;
	CaptureConsole *capture_console;

	Array<SideBarConsole*> consoles;
	void addConsole(SideBarConsole *c);
};

#endif /* BOTTOMBAR_H_ */
