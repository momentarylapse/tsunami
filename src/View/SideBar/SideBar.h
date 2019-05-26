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
class GlobalFxConsole;
class CurveConsole;
class SynthConsole;
class MidiFxConsole;
class CaptureConsole;
class AudioView;
class SideBarConsole;
class Session;

class SideBar : public Observable<hui::Panel>
{
public:
	SideBar(Session *session);
	virtual ~SideBar();

	static const int WIDTH_DEFAULT;
	static const int WIDTH_LARGE;

	void on_close();
	void on_large();
	void on_choose();

	bool is_large;
	void set_large(bool large);

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
		MIDI_FX_CONSOLE,
		SAMPLEREF_CONSOLE,
		CAPTURE_CONSOLE,
		NUM_CONSOLES
	};

	void choose(int console);
	void open(int console);
	bool is_active(int console);
	int active_console;
	bool visible;

	bool allow_close();

	SongConsole *song_console;
	GlobalFxConsole *global_fx_console;
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
	void add_console(SideBarConsole *c);

	Session *session;
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

	virtual void on_enter(){}
	virtual void on_leave(){}
	virtual void on_set_large(bool large){}
	virtual bool allow_close(){ return true; }
};

#endif /* BOTTOMBAR_H_ */
