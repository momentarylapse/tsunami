/*
 * InputStreamMidi.h
 *
 *  Created on: 19.02.2013
 *      Author: michi
 */

#ifndef INPUTSTREAMMIDI_H_
#define INPUTSTREAMMIDI_H_

#include "../Data/Song.h"
#include "../Midi/MidiSource.h"
#include "../Stuff/Observable.h"

class Device;
class DeviceManager;
class Session;

namespace hui{
	class Timer;
}

#if HAS_LIB_ALSA
struct _snd_seq_port_subscribe;
#endif

class InputStreamMidi : public Observable<VirtualBase>
{
public:

	InputStreamMidi(Session *session);
	virtual ~InputStreamMidi();

	void _cdecl init();

	static const string MESSAGE_CAPTURE;

	bool _cdecl start();
	void _cdecl stop();

	void _start_update();
	void _stop_update();
	void update();
	int do_capturing();

	bool _cdecl is_capturing();

	int _cdecl get_delay();
	void _cdecl reset_sync();

	void _cdecl accumulate(bool enable);
	void _cdecl reset_accumulation();
	int _cdecl get_sample_count();

	int _cdecl sample_rate(){ return _sample_rate; }

	bool _cdecl unconnect();
	void _cdecl set_device(Device *d);
	Device *_cdecl get_device();

	Session *session;

	MidiEventBuffer midi;
	MidiEventBuffer current_midi;

	class Output : public MidiSource
	{
	public:
		Output(InputStreamMidi *input);
		virtual ~Output();

		virtual int _cdecl read(MidiEventBuffer &midi);
		void _cdecl feed(const MidiEventBuffer &midi);

		MidiEventBuffer events;
		InputStreamMidi *input;
		bool real_time_mode;
	};
	Output *out;

	void _cdecl set_backup_mode(int mode);
	int backup_mode;

	void _cdecl set_chunk_size(int size);
	void _cdecl set_update_dt(float dt);
	int chunk_size;
	float update_dt;

	int _sample_rate;

private:

	void clear_input_queue();

#if HAS_LIB_ALSA
	_snd_seq_port_subscribe *subs;
#endif

	DeviceManager *device_manager;
	Device *device;
	int npfd;
	struct pollfd *pfd;
	hui::Timer *timer;
	double offset;
	bool capturing;
	bool accumulating;

	bool running;
	int hui_runner_id;
};

#endif /* INPUTSTREAMMIDI_H_ */
