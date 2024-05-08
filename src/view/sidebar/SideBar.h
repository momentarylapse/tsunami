/*
 * SideBar.h
 *
 *  Created on: 25.03.2014
 *      Author: michi
 */

#ifndef SIDEBAR_H_
#define SIDEBAR_H_

#include "../../lib/base/pointer.h"
#include "../../lib/hui/hui.h"
#include "../../lib/pattern/Observable.h"

class Song;
class SongConsole;
class TrackConsole;
class EffectsConsole;
class DummyEditorConsole;
class AudioEditorConsole;
class MidiEditorConsole;
class BarsEditorConsole;
class SampleRefConsole;
class SampleManagerConsole;
class CurveConsole;
class CaptureConsole;
class SignalChainPanel;
class AudioView;
class SideBarConsole;
class Session;

class SideBar : public obs::Node<hui::Panel> {
public:
	SideBar(Session *session, hui::Panel *parent);
	~SideBar() override;

	void on_close();
	void on_choose();

	void _show();
	void _hide();

	enum {
		SONG_CONSOLE,
		SAMPLE_CONSOLE,
		TRACK_CONSOLE,
		EFFECTS_CONSOLE,
		DUMMY_EDITOR_CONSOLE,
		AUDIO_EDITOR_CONSOLE,
		MIDI_EDITOR_CONSOLE,
		BARS_EDITOR_CONSOLE,
		CURVE_CONSOLE,
		SAMPLEREF_CONSOLE,
		CAPTURE_CONSOLE,
		SIGNAL_CHAIN_CONSOLE,
		NUM_CONSOLES
	};

	void choose(int console);
	void open(int console);
	bool is_active(int console);
	int active_console;
	bool visible;

	base::future<bool> test_allow_close();

	SongConsole *song_console;
	TrackConsole *track_console;
	EffectsConsole *effects_console;
	DummyEditorConsole *dummy_editor_console;
	AudioEditorConsole *audio_editor_console;
	MidiEditorConsole *midi_editor_console;
	BarsEditorConsole *bars_editor_console;
	CurveConsole *curve_console;
	SampleRefConsole *sample_ref_console;
	SampleManagerConsole *sample_manager;
	CaptureConsole *capture_console;
	SignalChainPanel *signal_chain_console;

	shared_array<SideBarConsole> consoles;
	void add_console(SideBarConsole *c);

	Session *session;
};


class SideBarConsole : public obs::Node<hui::Panel> {
public:
	SideBarConsole(const string &_title, const string &id, Session *_session, SideBar *bar);
	~SideBarConsole() override;
	string title;
	Session *session;
	Song *song;
	AudioView *view;
	SideBar *bar() { return dynamic_cast<SideBar*>(parent); }

	virtual void on_enter() {}
	virtual void on_leave() {}
	virtual base::future<bool> test_allow_close();
};

#endif /* BOTTOMBAR_H_ */
