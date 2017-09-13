/*
 * InputStreamMidi.h
 *
 *  Created on: 19.02.2013
 *      Author: michi
 */

#ifndef INPUTSTREAMMIDI_H_
#define INPUTSTREAMMIDI_H_

#include "../Data/Song.h"
#include "InputStreamAny.h"
#include "config.h"

class OutputStream;
class MidiRenderer;
class Synthesizer;
class Device;
class DeviceManager;
class MidiPreviewFeedSource;

namespace hui{
	class Timer;
}

#ifdef DEVICE_MIDI_ALSA
struct _snd_seq_port_subscribe;
#endif

class InputStreamMidi : public InputStreamAny
{
public:

	InputStreamMidi(int sample_rate);
	virtual ~InputStreamMidi();

	void _cdecl init();

	virtual bool _cdecl start();
	virtual void _cdecl stop();

	void _startUpdate();
	void _stopUpdate();
	void update();
	int doCapturing();

	virtual bool _cdecl isCapturing();

	virtual int _cdecl getDelay();
	virtual void _cdecl resetSync();

	virtual void _cdecl accumulate(bool enable);
	virtual void _cdecl resetAccumulation();
	virtual int _cdecl getSampleCount();

	virtual void _cdecl getSomeSamples(AudioBuffer &buf, int num_samples);
	virtual int _cdecl getState();

	virtual bool _cdecl unconnect();
	virtual void _cdecl setDevice(Device *d);
	virtual Device *_cdecl getDevice();

	void _cdecl setPreviewSynthesizer(Synthesizer *s);


	virtual int _cdecl getType(){ return Track::TYPE_MIDI; }

	MidiRawData midi;
	MidiRawData current_midi;

private:

	void clearInputQueue();

#ifdef DEVICE_MIDI_ALSA
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

	MidiPreviewFeedSource *preview_source;
	OutputStream *preview_stream;
	MidiRenderer *preview_renderer;
};

#endif /* INPUTSTREAMMIDI_H_ */
