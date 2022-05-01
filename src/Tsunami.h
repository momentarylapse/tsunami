/*
 * Tsunami.h
 *
 *  Created on: 13.08.2014
 *      Author: michi
 */

#ifndef TSUNAMI_H_
#define TSUNAMI_H_

#include "lib/base/pointer.h"
#include "lib/hui/hui.h"

extern const string AppName;
extern const string AppVersion;


class Song;
class PluginManager;
class Slider;
class Log;
class AudioInput;
class DeviceManager;
class AudioView;
class Storage;
class Clipboard;
class TsunamiWindow;
class Session;
class PerformanceMonitor;

class Tsunami : public hui::Application {
public:
	Tsunami();
	virtual ~Tsunami();

	bool on_startup(const Array<string> &arg) override;
	void on_end() override;

	bool handle_arguments(const Array<string> &arg);
	void load_key_codes();

	void test_allow_termination(hui::Callback cb_yes, hui::Callback cb_no);

	shared_array<Session> sessions;

	owned<Log> log;

	owned<DeviceManager> device_manager;
	owned<PluginManager> plugin_manager;
	owned<Clipboard> clipboard;
	owned<PerformanceMonitor> perf_mon;
};


extern Tsunami *tsunami;

#endif /* TSUNAMI_H_ */
