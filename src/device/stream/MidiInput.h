/*
 * MidiInput.h
 *
 *  Created on: 19.02.2013
 *      Author: michi
 */

#ifndef SRC_DEVICE_STREAM_MIDIINPUT_H_
#define SRC_DEVICE_STREAM_MIDIINPUT_H_

#include "../interface/MidiInputStream.h"
#include "../../data/midi/MidiData.h"
#include "../../module/port/Port.h"
#include "../../module/Module.h"
#include "../../module/ModuleConfiguration.h"

class Device;
class DeviceManager;
class Session;
namespace os {
	class Timer;
}

class MidiInput : public Module {
public:

	MidiInput(Session *session);
	virtual ~MidiInput();

	void _create_dev();
	void _kill_dev();

	enum class State {
		NO_DEVICE,
		CAPTURING,
		PAUSED,
	} state;

	bool _cdecl start();
	void _cdecl stop();

	int do_capturing();

	bool _cdecl is_capturing() const;

	int _cdecl get_delay();
	void _cdecl reset_sync();

	int _cdecl sample_rate() { return _sample_rate; }

	bool _cdecl unconnect();
	void _cdecl set_device(Device *d);
	Device *_cdecl get_device();

	MidiEventBuffer current_midi;

	MidiOutPort out{this};

	int read_midi(int port, MidiEventBuffer &midi) override;
	void feed(const MidiEventBuffer &midi);

	MidiEventBuffer events;

	int _sample_rate;

private:
	MidiInputStream* stream = nullptr;

	MidiInputStream::SharedData shared_data;


	DeviceManager *device_manager;
	Device *cur_device;
	void update_device();


	class Config : public ModuleConfiguration {
	public:
		Device *device;
		bool free_flow;
		void reset() override;
		string auto_conf(const string &name) const override;
	} config;

	ModuleConfiguration* get_config() const override;
	void on_config() override;

	int npfd;
	struct pollfd *pfd;
	os::Timer *timer;
	double offset;

public:
	base::optional<int64> command(ModuleCommand cmd, int64 param) override;
};

#endif /* SRC_DEVICE_STREAM_MIDIINPUT_H_ */
