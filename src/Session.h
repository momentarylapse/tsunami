/*
 * Session.h
 *
 *  Created on: 09.03.2018
 *      Author: michi
 */

#ifndef SRC_SESSION_H_
#define SRC_SESSION_H_

#include "lib/base/base.h"
#include "Stuff/Observable.h"

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
class SongRenderer;
class PeakMeter;
class AudioOutput;
namespace hui {
	class Window;
}

// representing one instance/window
class Session : public Observable<VirtualBase> {
public:
	Session(Log *log, DeviceManager *device_manager, PluginManager *plugin_manager, PerformanceMonitor *perf_mon);
	virtual ~Session();

	static const string MESSAGE_ADD_PLUGIN;
	static const string MESSAGE_REMOVE_PLUGIN;
	static const string MESSAGE_ADD_SIGNAL_CHAIN;

	static int next_id;

	void set_win(TsunamiWindow *win);

	Session *create_child();

	int id;
	TsunamiWindow *win;
	hui::Window *_kaba_win;
	Song *song;
	AudioView *view;
	Storage *storage;

	string storage_options;

	SignalChain *signal_chain;
	SongRenderer *song_renderer;
	PeakMeter *peak_meter;
	AudioOutput *output_stream;
	Array<SignalChain*> all_signal_chains;
	SignalChain* add_signal_chain(const string &name);
	SignalChain* add_signal_chain_system(const string &name);
	void _remove_signal_chain(SignalChain *chain);

	Array<TsunamiPlugin*> plugins;
	TsunamiPlugin *last_plugin;
	bool die_on_plugin_stop;


	void execute_tsunami_plugin(const string &name);
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
