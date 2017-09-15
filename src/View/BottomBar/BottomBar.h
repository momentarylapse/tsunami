/*
 * BottomBar.h
 *
 *  Created on: 23.03.2014
 *      Author: michi
 */

#ifndef BOTTOMBAR_H_
#define BOTTOMBAR_H_

#include "../../lib/hui/hui.h"
#include "../../Stuff/Observable.h"

class AudioView;
class Song;
class DeviceManager;
class Track;
class MixingConsole;
class LogConsole;
class DeviceConsole;
class Log;
class BottomBarConsole;

class BottomBar : public Observable<hui::Panel>
{
public:
	BottomBar(AudioView *view, Song *audio, DeviceManager *device_manager, Log *log);
	virtual ~BottomBar();


	enum{
		LOG_CONSOLE,
		MIXING_CONSOLE,
		DEVICE_CONSOLE
	};


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
	void open(int console_index);
	bool isActive(int console_index);
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
