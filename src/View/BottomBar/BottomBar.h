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
class Song;
class AudioOutput;
class Track;
class MixingConsole;
class CurveConsole;
class LogDialog;
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
	BottomBar(AudioView *view, Song *audio, AudioOutput *output, Log *log);
	virtual ~BottomBar();

	void onClose();
	void onChoose();

	void _show();
	void _hide();

	enum
	{
		LOG_CONSOLE,
		MIXING_CONSOLE,
		CURVE_CONSOLE,
		NUM_CONSOLES
	};

	void choose(int console);
	void open(int console);
	bool isActive(int console);
	int active_console;
	bool visible;

	MixingConsole *mixing_console;
	CurveConsole *curve_console;
	LogDialog *log_dialog;

	Array<BottomBarConsole*> consoles;
	void addConsole(BottomBarConsole *c, const string &list_name);
};

#endif /* BOTTOMBAR_H_ */
