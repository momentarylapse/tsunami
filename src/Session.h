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
class PerformanceMonitor;
class SignalChain;
class SongRenderer;
class PeakMeter;
class OutputStream;
namespace hui{
	class Window;
}

// representing one instance/window
class Session : public VirtualBase
{
public:
	Session(Log *log, DeviceManager *device_manager, PluginManager *plugin_manager, PerformanceMonitor *perf_mon);
	virtual ~Session();

	static int next_id;

	void setWin(TsunamiWindow *win);

	int id;
	TsunamiWindow *win;
	hui::Window *_kaba_win;
	Song *song;
	AudioView *view;
	Storage *storage;

	SignalChain *signal_chain;
	SongRenderer *song_renderer;
	PeakMeter *peak_meter;
	OutputStream *output_stream;

	Array<TsunamiPlugin*> plugins;
	bool die_on_plugin_stop;


	void executeTsunamiPlugin(const string &name);
	void onPluginStopRequest(VirtualBase *o);

	int sample_rate();

	// global
	Log *log;
	DeviceManager *device_manager;
	PluginManager *plugin_manager;
	PerformanceMonitor *perf_mon;


	// logging
	void i(const string &msg);
	void w(const string &msg);
	void e(const string &msg);

	static Session *GLOBAL;
};

#endif /* SRC_SESSION_H_ */
