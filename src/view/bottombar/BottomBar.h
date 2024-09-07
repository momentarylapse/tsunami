/*
 * BottomBar.h
 *
 *  Created on: 23.03.2014
 *      Author: michi
 */

#ifndef SRC_VIEW_BOTTOMBAR_BOTTOMBAR_H_
#define SRC_VIEW_BOTTOMBAR_BOTTOMBAR_H_

#include "../../lib/base/pointer.h"
#include "../../lib/hui/Panel.h"
#include "../../lib/pattern/Observable.h"

namespace tsunami {

class AudioView;
class Song;
class DeviceManager;
class Track;
class MixingConsole;
class LogConsole;
class SignalChainConsole;
class Session;
class BottomBarConsole;
class PluginConsole;
class SessionConsole;
class DeviceConsole;

class BottomBar : public obs::Node<hui::Panel> {
public:
	BottomBar(Session *session, hui::Panel *parent);


	enum class Index {
		MixingConsole,
	//	SignalEditor,
		SignalChainConsole,
		PluginConsole,
		DeviceConsole,
		SessionConsole,
		LogConsole
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
	void open(Index console_index);
	void toggle(Index console_index);
	bool is_active(Index console_index) const;
	Console *active_console;
	bool visible;

	MixingConsole *mixing_console;
	SignalChainConsole *signal_chain_console;
	PluginConsole *plugin_console;
	DeviceConsole *device_console;
	SessionConsole *session_console;
	LogConsole *log_console;

	Index index(Console *console);

	shared_array<Console> consoles;
	void add_console(Console *c, const string &list_name);
};

}

#endif /* SRC_VIEW_BOTTOMBAR_BOTTOMBAR_H_ */
