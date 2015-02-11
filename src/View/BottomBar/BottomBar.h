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
class LevelConsole;
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

	enum
	{
		LOG_CONSOLE,
		AUDIOFILE_CONSOLE,
		MIXING_CONSOLE,
		LEVEL_CONSOLE,
		SAMPLE_CONSOLE,
		CURVE_CONSOLE,
		TRACK_CONSOLE,
		FX_CONSOLE,
		SYNTH_CONSOLE,
		MIDI_EDITOR,
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
	TrackConsole *track_console;
	FxConsole *fx_console;
	SynthConsole *synth_console;
	MixingConsole *mixing_console;
	LevelConsole *level_console;
	CurveConsole *curve_console;
	LogDialog *log_dialog;
	SampleManager *sample_manager;
	MidiEditor *midi_editor;

	Array<BottomBarConsole*> consoles;
	void addConsole(BottomBarConsole *c, const string &list_name);
};

#endif /* BOTTOMBAR_H_ */
