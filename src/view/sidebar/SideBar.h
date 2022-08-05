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
#include "../../stuff/Observable.h"

class Song;
class SongConsole;
class TrackConsole;
class DummyEditorConsole;
class AudioEditorConsole;
class MidiEditorConsole;
class SampleRefConsole;
class SampleManagerConsole;
class CurveConsole;
class CaptureConsole;
class AudioView;
class SideBarConsole;
class Session;

class SideBar : public Observable<hui::Panel> {
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
		DUMMY_EDITOR_CONSOLE,
		AUDIO_EDITOR_CONSOLE,
		MIDI_EDITOR_CONSOLE,
		CURVE_CONSOLE,
		SAMPLEREF_CONSOLE,
		CAPTURE_CONSOLE,
		NUM_CONSOLES
	};

	void choose(int console);
	void open(int console);
	bool is_active(int console);
	int active_console;
	bool visible;

	void test_allow_close(hui::Callback cb_yes, hui::Callback cb_no);

	SongConsole *song_console;
	TrackConsole *track_console;
	DummyEditorConsole *dummy_editor_console;
	AudioEditorConsole *audio_editor_console;
	MidiEditorConsole *midi_editor_console;
	CurveConsole *curve_console;
	SampleRefConsole *sample_ref_console;
	SampleManagerConsole *sample_manager;
	CaptureConsole *capture_console;

	shared_array<SideBarConsole> consoles;
	void add_console(SideBarConsole *c);

	Session *session;
};


class SideBarConsole : public hui::Panel {
public:
	SideBarConsole(const string &_title, const string &id, Session *_session, SideBar *bar);
	string title;
	Session *session;
	Song *song;
	AudioView *view;
	SideBar *bar() { return dynamic_cast<SideBar*>(parent); }

	virtual void on_enter() {}
	virtual void on_leave() {}
	virtual void on_set_large(bool large) {}
	virtual void test_allow_close(hui::Callback cb_yes, hui::Callback cb_no) { cb_yes(); }
};

#endif /* BOTTOMBAR_H_ */