/*
 * BottomBar.h
 *
 *  Created on: 23.03.2014
 *      Author: michi
 */

#ifndef SRC_VIEW_BOTTOMBAR_BOTTOMBAR_H_
#define SRC_VIEW_BOTTOMBAR_BOTTOMBAR_H_

#include "../../lib/base/pointer.h"
#include "../../lib/hui/hui.h"
#include "../../lib/pattern/Observable.h"

class AudioView;
class Song;
class DeviceManager;
class Track;
class MixingConsole;
class LogConsole;
class SignalEditor;
class SignalChainConsole;
class Session;
class BottomBarConsole;
class PluginConsole;
class SessionConsole;
class DeviceConsole;

class BottomBar : public obs::Node<hui::Panel> {
public:
	BottomBar(Session *session, hui::Panel *parent);


	enum {
		MIXING_CONSOLE,
	//	SIGNAL_EDITOR,
		SIGNAL_CHAIN_CONSOLE,
		PLUGIN_CONSOLE,
		DEVICE_CONSOLE,
		SESSION_CONSOLE,
		LOG_CONSOLE
	};


	class Console : public obs::Node<hui::Panel> {
	public:
		Console(const string &_title, const string &id, Session *_session, BottomBar *bar);
		string title;
		Session *session;
		Song *song;
		AudioView *view;
		BottomBar *bar(){ return dynamic_cast<BottomBar*>(parent); }


		virtual void on_enter() {}
		virtual void on_leave() {}
		void blink();
		bool request_notify;
	};

	void on_close();
	void on_choose();

	void _show();
	void _hide();

	void choose(Console *console);
	void open(Console *console);
	void open(int console_index);
	void toggle(int console_index);
	bool is_active(int console_index);
	Console *active_console;
	bool visible;

	MixingConsole *mixing_console;
	//SignalEditor *signal_editor;
	SignalChainConsole *signal_chain_console;
	PluginConsole *plugin_console;
	DeviceConsole *device_console;
	SessionConsole *session_console;
	LogConsole *log_console;

	int index(Console *console);

	shared_array<Console> consoles;
	void add_console(Console *c, const string &list_name);
};

#endif /* SRC_VIEW_BOTTOMBAR_BOTTOMBAR_H_ */
