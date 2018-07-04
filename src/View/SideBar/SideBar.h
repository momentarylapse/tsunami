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

class Song;
class SongConsole;
class TrackConsole;
class MidiEditorConsole;
class SampleRefConsole;
class SampleManagerConsole;
class FxConsole;
class CurveConsole;
class SynthConsole;
class MidiFxConsole;
class CaptureConsole;
class ModuleConsole;
class AudioView;
class SideBarConsole;
class Session;

class SideBar : public Observable<hui::Panel>
{
public:
	SideBar(Session *session);
	virtual ~SideBar();

	void onClose();
	void onChoose();

	void _show();
	void _hide();

	enum
	{
		SONG_CONSOLE,
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
		MODULE_CONSOLE,
		NUM_CONSOLES
	};

	void choose(int console);
	void open(int console);
	bool isActive(int console);
	int active_console;
	bool visible;

	SongConsole *song_console;
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
	ModuleConsole *module_console;

	Array<SideBarConsole*> consoles;
	void addConsole(SideBarConsole *c);
};


class SideBarConsole : public hui::Panel
{
public:
	SideBarConsole(const string &_title, Session *_session);
	string title;
	Session *session;
	Song *song;
	AudioView *view;
	SideBar *bar(){ return dynamic_cast<SideBar*>(parent); }

	virtual void onEnter(){}
	virtual void onLeave(){}
};

#endif /* BOTTOMBAR_H_ */
