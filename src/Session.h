/*
 * Session.h
 *
 *  Created on: 09.03.2018
 *      Author: michi
 */

#ifndef SRC_SESSION_H_
#define SRC_SESSION_H_

#include "lib/base/base.h"
#include "lib/base/pointer.h"
#include "lib/pattern/Observable.h"

class TsunamiWindow;
class Log;
class Song;
class Storage;
class AudioView;
class TsunamiPlugin;
class DeviceManager;
class PluginManager;
class PerformanceMonitor;
class SignalChain;
class Path;
namespace hui {
	class Window;
}


// representing one instance/window
class Session : public Sharable<Observable<VirtualBase>> {
public:
	Session(Log *log, DeviceManager *device_manager, PluginManager *plugin_manager, PerformanceMonitor *perf_mon);
	~Session() override;

	static const string MESSAGE_ADD_PLUGIN;
	static const string MESSAGE_REMOVE_PLUGIN;
	static const string MESSAGE_ADD_SIGNAL_CHAIN;

	static int next_id;

	void set_win(TsunamiWindow *win);

	xfer<Session> create_child();

	void prepare_end();

	int id;
	owned<TsunamiWindow> win;
	hui::Window *_kaba_win;
	shared<Song> song;
	AudioView *view;
	owned<Storage> storage;
	bool auto_delete;

	shared_array<SignalChain> all_signal_chains;
	void add_signal_chain(xfer<SignalChain> chain);
	shared<SignalChain> create_signal_chain(const string &name);
	shared<SignalChain> create_signal_chain_system(const string &name);
	shared<SignalChain> load_signal_chain(const Path &filename);
	void remove_signal_chain(SignalChain *chain);

	shared_array<TsunamiPlugin> plugins;
	TsunamiPlugin *last_plugin;
	bool die_on_plugin_stop;


	shared<TsunamiPlugin> execute_tsunami_plugin(const string &name, const Array<string> &args = {});
	void on_plugin_stop_request(TsunamiPlugin *p);

	int sample_rate();

	void set_mode(const string &mode);
	bool in_mode(const string &mode);
	string mode;

	// global
	Log *log;
	DeviceManager *device_manager;
	PluginManager *plugin_manager;
	PerformanceMonitor *perf_mon;


	// logging
	void i(const string &msg);
	void w(const string &msg);
	void e(const string &msg);
	void q(const string &msg, const Array<string> &responses);
	void debug(const string &cat, const string &msg);

	static Session *GLOBAL;
};

#endif /* SRC_SESSION_H_ */
