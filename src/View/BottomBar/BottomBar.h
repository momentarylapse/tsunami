/*
 * BottomBar.h
 *
 *  Created on: 23.03.2014
 *      Author: michi
 */

#ifndef BOTTOMBAR_H_
#define BOTTOMBAR_H_

#include "../../lib/hui/hui.h"
#include "../../Stuff/Observer.h"

class AudioView;
class AudioFile;
class AudioFileConsole;
class AudioOutput;
class Track;
class TrackConsole;
class FxConsole;
class SynthConsole;
class MixingConsole;
class CurveConsole;
class LogDialog;
class SampleManager;
class MidiEditor;
class Log;

class BottomBarConsole : public HuiPanel
{
public:
	BottomBarConsole(const string &_title)
	{ title = _title; }
	string title;
};

class BottomBar : public HuiPanel, public Observable
{
public:
	BottomBar(AudioView *view, AudioFile *audio, AudioOutput *output, Log *log);
	virtual ~BottomBar();

	void onClose();
	void onChoose();
	virtual void onShow();
	virtual void onHide();

	void show();
	void hide();

	enum
	{
		LOG_CONSOLE,
		AUDIOFILE_CONSOLE,
		MIXING_CONSOLE,
		FX_CONSOLE,
		SAMPLE_CONSOLE,
		CURVE_CONSOLE,
		TRACK_CONSOLE,
		TRACK_FX_CONSOLE,
		TRACK_SYNTH_CONSOLE,
		TRACK_MIDI_EDITOR,
		NUM_CONSOLES
	};

	void choose(int console);
	bool isActive(int console);
	int active_console;
	bool visible;

	bool ready;
	int console_when_ready;

	//HuiMenu *menu;
	AudioFileConsole *audio_file_console;
	FxConsole *fx_console;
	TrackConsole *track_console;
	FxConsole *track_fx_console;
	SynthConsole *track_synth_console;
	MixingConsole *mixing_console;
	CurveConsole *curve_console;
	LogDialog *log_dialog;
	SampleManager *sample_manager;
	MidiEditor *track_midi_editor;

	Array<BottomBarConsole*> consoles;
	void addConsole(BottomBarConsole *c, const string &list_name);
};

#endif /* BOTTOMBAR_H_ */
