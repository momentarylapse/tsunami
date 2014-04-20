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
class CurveConsole;
class LogDialog;
class SampleManager;
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
	void OnOpenChooseMenu();
	void OnChooseByMenu();
	virtual void OnShow();
	virtual void OnHide();

	enum
	{
		MIXING_CONSOLE,
		FX_CONSOLE,
		SYNTH_CONSOLE,
		SAMPLE_CONSOLE,
		CURVE_CONSOLE,
		LOG_CONSOLE,
		NUM_CONSOLES
	};

	void Choose(int console);
	bool IsActive(int console);
	int active_console;
	bool visible;

	HuiMenu *menu;
	FxConsole *fx_console;
	SynthConsole *synth_console;
	MixingConsole *mixing_console;
	CurveConsole *curve_console;
	LogDialog *log_dialog;
	SampleManager *sample_manager;
};

#endif /* BOTTOMBAR_H_ */
