/*
 * InputStreamMidi.h
 *
 *  Created on: 19.02.2013
 *      Author: michi
 */

#ifndef INPUTSTREAMMIDI_H_
#define INPUTSTREAMMIDI_H_

#include "../lib/base/base.h"
#include "../lib/hui/hui.h"
#include "../Data/Song.h"
#include "../View/Helper/PeakMeter.h"
#include "config.h"

class OutputStream;
class MidiRenderer;
class Synthesizer;
class Device;
class DeviceManager;
class MidiPreviewFeedSource;

#ifdef DEVICE_MIDI_ALSA
struct _snd_seq_port_subscribe;
#endif

class InputStreamMidi : public PeakMeterSource
{
public:

	InputStreamMidi(int sample_rate);
	virtual ~InputStreamMidi();


	static const string MESSAGE_CAPTURE;

	void _cdecl init();

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

	virtual float _cdecl getSampleRate();
	virtual void _cdecl getSomeSamples(BufferBox &buf, int num_samples);
	virtual int _cdecl getState();

	bool _cdecl unconnect();
	void _cdecl setDevice(Device *d);
	Device *_cdecl getDevice();

	void _cdecl setPreviewSynthesizer(Synthesizer *s);

	void _cdecl setChunkSize(int size);
	void _cdecl setUpdateDt(float dt);
	int chunk_size;
	float update_dt;


	int sample_rate;

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
	HuiTimer timer;
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
