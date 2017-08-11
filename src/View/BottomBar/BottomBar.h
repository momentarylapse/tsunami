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




	class Console : public hui::Panel
	{
	public:
		Console(const string &_title)
		{ title = _title; notify = false; }
		string title;
		BottomBar *bar(){ return dynamic_cast<BottomBar*>(parent); }


		void blink();
		bool notify;
	};

	void onClose();
	void onChoose();

	void _show();
	void _hide();

	void choose(Console *console);
	void open(Console *console);
	bool isActive(Console *console);
	Console *active_console;
	bool visible;

	MixingConsole *mixing_console;
	LogConsole *log_console;
	DeviceConsole *device_console;

	int index(Console *console);

	Array<Console*> consoles;
	void addConsole(Console *c, const string &list_name);
};

#endif /* BOTTOMBAR_H_ */
