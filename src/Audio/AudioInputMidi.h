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
#include "../Data/AudioFile.h"
#include "AudioInputBase.h"

struct _snd_seq;
struct _snd_seq_port_subscribe;

class AudioInputMidi : public AudioInputBase
{
public:
	AudioInputMidi(MidiData &data);
	virtual ~AudioInputMidi();

	void Init();

	virtual bool Start(int sample_rate);
	virtual void Stop();
	virtual int DoCapturing();

	virtual bool IsCapturing();

	virtual int GetDelay();
	virtual void ResetSync();

	virtual void Accumulate(bool enable);
	virtual void ResetAccumulation();
	virtual int GetSampleCount();

	virtual float GetSampleRate();
	virtual BufferBox GetSomeSamples(int num_samples);

	struct MidiPort
	{
		int client, port;
		string client_name, port_name;
	};
	Array<MidiPort> FindPorts();
	bool ConnectTo(MidiPort p);
	bool Unconnect();

private:

	void ClearInputQueue();

	MidiData &data;

	_snd_seq *handle;
	_snd_seq_port_subscribe *subs;
	int portid;
	int npfd;
	struct pollfd *pfd;
	int sample_rate;
	HuiTimer timer;
	double offset;
	bool capturing;
	bool accumulating;

	int tone_start[128];
	float tone_volume[128];
};

#endif /* AUDIOINPUTMIDI_H_ */
