/*
 * InputStreamMidi.h
 *
 *  Created on: 19.02.2013
 *      Author: michi
 */

#ifndef INPUTSTREAMMIDI_H_
#define INPUTSTREAMMIDI_H_

#include "../Data/Song.h"
#include "../View/Helper/PeakMeter.h"
#include "../Midi/MidiSource.h"

class Device;
class DeviceManager;
class Session;

namespace hui{
	class Timer;
}

#if HAS_LIB_ALSA
struct _snd_seq_port_subscribe;
#endif

class InputStreamMidi : public PeakMeterSource
{
public:

	InputStreamMidi(Session *session, int sample_rate);
	virtual ~InputStreamMidi();

	void _cdecl init();

	static const string MESSAGE_CAPTURE;

	bool _cdecl start();
	void _cdecl stop();

	void _startUpdate();
	void _stopUpdate();
	void update();
	int doCapturing();

	bool _cdecl isCapturing();

	int _cdecl getDelay();
	void _cdecl resetSync();

	void _cdecl accumulate(bool enable);
	void _cdecl resetAccumulation();
	int _cdecl getSampleCount();

	virtual float _cdecl getSampleRate(){ return sample_rate; }
	virtual void _cdecl getSomeSamples(AudioBuffer &buf, int num_samples);
	virtual int _cdecl getState();

	bool _cdecl unconnect();
	void _cdecl setDevice(Device *d);
	Device *_cdecl getDevice();

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

	void _cdecl setBackupMode(int mode);
	int backup_mode;

	void _cdecl setChunkSize(int size);
	void _cdecl setUpdateDt(float dt);
	int chunk_size;
	float update_dt;

	int sample_rate;

private:

	void clearInputQueue();

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
