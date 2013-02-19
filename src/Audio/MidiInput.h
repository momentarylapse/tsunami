/*
 * MidiInput.h
 *
 *  Created on: 19.02.2013
 *      Author: michi
 */

#ifndef MIDIINPUT_H_
#define MIDIINPUT_H_

#include "../lib/base/base.h"
#include "../lib/hui/hui.h"
#include "../Data/AudioFile.h"
#include "../View/Helper/PeakMeter.h"

struct _snd_seq;

class MidiInput : public HuiEventHandler, public PeakMeterSource
{
public:
	MidiInput();
	virtual ~MidiInput();

	void Init();

	bool Start(int sample_rate);
	void Stop();
	void Update();

	bool IsCapturing();

	virtual float GetSampleRate();
	virtual BufferBox GetSomeSamples(int num_samples);

	MidiData data;

private:
	int DoCapturing();

	_snd_seq *handle;
	int npfd;
	struct pollfd *pfd;
	int sample_rate;
};

#endif /* MIDIINPUT_H_ */
