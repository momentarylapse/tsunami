/*
 * MidiInput.h
 *
 *  Created on: 19.02.2013
 *      Author: michi
 */

#ifndef SRC_STREAM_MIDIINPUT_H_
#define SRC_STREAM_MIDIINPUT_H_

#include "../Data/Midi/MidiData.h"
#include "../Module/Port/Port.h"
#include "../Module/Module.h"
#include "../Module/ModuleConfiguration.h"

class Device;
class DeviceManager;
class Session;

namespace hui {
	class Timer;
}

#if HAS_LIB_ALSA
struct _snd_seq_port_subscribe;
#endif

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

	bool _cdecl is_capturing();

	int _cdecl get_delay();
	void _cdecl reset_sync();

	int _cdecl sample_rate(){ return _sample_rate; }

	bool _cdecl unconnect();
	void _cdecl set_device(Device *d);
	Device *_cdecl get_device();

	MidiEventBuffer current_midi;

	class Output : public Port {
	public:
		Output(MidiInput *input);

		int read_midi(MidiEventBuffer &midi) override;
		void feed(const MidiEventBuffer &midi);

		MidiEventBuffer events;
		MidiInput *input;
	};
	Output *out;

	int _sample_rate;

private:

	void clear_input_queue();

#if HAS_LIB_ALSA
	_snd_seq_port_subscribe *subs;
#endif
	int portid;

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
	hui::Timer *timer;
	double offset;

public:
	int command(ModuleCommand cmd, int param) override;
};

#endif /* SRC_STREAM_MIDIINPUT_H_ */
