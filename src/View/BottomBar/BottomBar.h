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
class AudioOutput;
class Track;
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

	void OnClose();
	void OnChoose();
	//void OnOpenChooseMenu();
	//void OnChooseByMenu();
	virtual void OnShow();
	virtual void OnHide();

	enum
	{
		MIXING_CONSOLE,
		FX_CONSOLE,
		SYNTH_CONSOLE,
		MIDI_EDITOR,
		LEVEL_CONSOLE,
		SAMPLE_CONSOLE,
		CURVE_CONSOLE,
		LOG_CONSOLE,
		NUM_CONSOLES
	};

	void Choose(int console);
	bool IsActive(int console);
	int active_console;
	bool visible;

	//HuiMenu *menu;
	FxConsole *fx_console;
	SynthConsole *synth_console;
	MixingConsole *mixing_console;
	LevelConsole *level_console;
	CurveConsole *curve_console;
	LogDialog *log_dialog;
	SampleManager *sample_manager;
	MidiEditor *midi_editor;
};

#endif /* BOTTOMBAR_H_ */
