/*
 * InputStreamAny.h
 *
 *  Created on: 16.08.2015
 *      Author: michi
 */

#ifndef SRC_AUDIO_AUDIOINPUTANY_H_
#define SRC_AUDIO_AUDIOINPUTANY_H_

#include "../lib/base/base.h"
#include "../lib/hui/hui.h"
#include "../Data/RingBuffer.h"
#include "../Data/Song.h"
#include "../View/Helper/PeakMeter.h"

class InputStreamAudio;
class InputStreamMidi;
class Device;

class InputStreamAny : public PeakMeterSource, public Observer
{
public:

	InputStreamAny(int sample_rate);
	virtual ~InputStreamAny();

	static const string MESSAGE_CAPTURE;

	void setType(int type);

	bool start();
	void stop();

	bool isCapturing();
	int getDelay(){ return 0; }
	void resetSync(){}

	void accumulate(bool enable);
	void resetAccumulation();
	int getSampleCount();

	virtual float _cdecl getSampleRate(){ return sample_rate; }
	virtual void _cdecl getSomeSamples(BufferBox &buf, int num_samples);
	virtual int _cdecl getState();

	void setDevice(Device *d);
	Device *getDevice();

	void setBackupMode(int mode);

	void setPreviewSynthesizer(Synthesizer *s);

	virtual void onUpdate(Observable *o, const string &message);


	void _cdecl setChunkSize(int size);
	void _cdecl setUpdateDt(float dt);
	int chunk_size;
	float update_dt;

	int sample_rate;

	RingBuffer *current_buffer;
	BufferBox *buffer;
	MidiRawData *midi;
	MidiRawData *current_midi;

	int type;
	InputStreamAudio *input_audio;
	InputStreamMidi *input_midi;
	int backup_mode;

	Synthesizer *preview_synth;
};

#endif /* SRC_AUDIO_AUDIOINPUTANY_H_ */
