/*
 * InputStreamMidi.h
 *
 *  Created on: 19.02.2013
 *      Author: michi
 */

#ifndef INPUTSTREAMMIDI_H_
#define INPUTSTREAMMIDI_H_

#include "../Data/Midi/MidiData.h"
#include "../Module/Port/Port.h"
#include "../Module/Module.h"

class Device;
class DeviceManager;
class Session;

namespace hui{
	class Timer;
}

#if HAS_LIB_ALSA
struct _snd_seq_port_subscribe;
#endif

class InputStreamMidi : public Module
{
public:

	InputStreamMidi(Session *session);
	virtual ~InputStreamMidi();

	void _create_dev();
	void _kill_dev();

	enum class State{
		NO_DEVICE,
		CAPTURING,
		PAUSED,
	} state;

	void init();

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

	class Output : public Port
	{
	public:
		Output(InputStreamMidi *input);

		int read_midi(MidiEventBuffer &midi) override;
		void feed(const MidiEventBuffer &midi);

		MidiEventBuffer events;
		InputStreamMidi *input;
		bool real_time_mode;
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
	Device *device;
	int npfd;
	struct pollfd *pfd;
	hui::Timer *timer;
	double offset;

public:
	int command(ModuleCommand cmd, int param) override;
};

#endif /* INPUTSTREAMMIDI_H_ */
