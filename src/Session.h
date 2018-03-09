/*
 * Session.h
 *
 *  Created on: 09.03.2018
 *      Author: michi
 */

#ifndef SRC_SESSION_H_
#define SRC_SESSION_H_

#include "lib/base/base.h"

class TsunamiWindow;
class Log;
class Song;
class Storage;
class AudioView;
class TsunamiPlugin;
class DeviceManager;
class PluginManager;
namespace hui{
	class Window;
}

// representing one instance/window
class Session : public VirtualBase
{
public:
	Session();
	virtual ~Session();

	static int next_id;

	void setWin(TsunamiWindow *win);

	int id;
	TsunamiWindow *win;
	hui::Window *_kaba_win;
	Song *song;
	AudioView *view;
	Storage *storage;

	Array<TsunamiPlugin*> plugins;
	bool die_on_plugin_stop;


	void executeTsunamiPlugin(const string &name);
	void onPluginStopRequest(VirtualBase *o);


	// global
	Log *log;
	DeviceManager *device_manager;
	PluginManager *plugin_manager;


	// logging
	void i(const string &msg);
	void w(const string &msg);
	void e(const string &msg);

	static Session *GLOBAL;
};

#endif /* SRC_SESSION_H_ */
