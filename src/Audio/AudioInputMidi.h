/*
 * AudioInputMidi.h
 *
 *  Created on: 19.02.2013
 *      Author: michi
 */

#ifndef AUDIOINPUTMIDI_H_
#define AUDIOINPUTMIDI_H_

#include "../lib/base/base.h"
#include "../lib/hui/hui.h"
#include "../Data/Song.h"
#include "../View/Helper/PeakMeter.h"

struct _snd_seq_port_subscribe;
class AudioStream;
class SynthesizerRenderer;
class Synthesizer;
class Device;
class DeviceManager;

class AudioInputMidi : public PeakMeterSource
{
public:

	AudioInputMidi(int sample_rate);
	virtual ~AudioInputMidi();


	static const string MESSAGE_CAPTURE;

	void init();

	bool start();
	void stop();

	void _startUpdate();
	void _stopUpdate();
	void update();
	int doCapturing();

	bool isCapturing();

	int getDelay();
	void resetSync();

	void accumulate(bool enable);
	void resetAccumulation();
	int getSampleCount();

	virtual float getSampleRate();
	virtual void getSomeSamples(BufferBox &buf, int num_samples);
	virtual int getState();

	bool unconnect();
	void _cdecl setDevice(Device *d);
	Device *_cdecl getDevice();

	void setPreviewSynthesizer(Synthesizer *s);

	void _cdecl setChunkSize(int size);
	void _cdecl setUpdateDt(float dt);
	int chunk_size;
	float update_dt;


	int sample_rate;

	MidiRawData midi;
	MidiRawData current_midi;

private:

	void clearInputQueue();

	_snd_seq_port_subscribe *subs;
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

	AudioStream *preview_stream;
	SynthesizerRenderer *preview_renderer;
};

#endif /* AUDIOINPUTMIDI_H_ */
