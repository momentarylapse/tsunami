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
class DeviceManager;
class Track;
class MixingConsole;
class LogConsole;
class DeviceConsole;
class Log;
class BottomBarConsole;

class BottomBar : public hui::Panel, public Observable
{
public:
	BottomBar(AudioView *view, Song *audio, DeviceManager *device_manager, Log *log);
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
	LogConsole *log_console;
	DeviceConsole *device_console;

	Array<BottomBarConsole*> consoles;
	void addConsole(BottomBarConsole *c, const string &list_name);
};


class BottomBarConsole : public hui::Panel
{
public:
	BottomBarConsole(const string &_title)
	{ title = _title; }
	string title;
	BottomBar *bar(){ return dynamic_cast<BottomBar*>(parent); }
};

#endif /* BOTTOMBAR_H_ */
