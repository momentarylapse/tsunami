/*
 * AudioOutput.h
 *
 *  Created on: 26.03.2012
 *      Author: michi
 */

#ifndef AUDIOOUTPUT_H_
#define AUDIOOUTPUT_H_

#include "../lib/base/base.h"
#include "../lib/hui/hui.h"
#include "../Data/AudioFile.h"
#include "../Stuff/PeakMeter.h"

class AudioOutput : public HuiEventHandler, public PeakMeterSource
{
public:
	AudioOutput();
	virtual ~AudioOutput();

	Array<string> Device;
	string ChosenDevice;
	bool PlayLoop;

	void Init();
	void Kill();

	void Stop();
	void Play(AudioFile *a, bool loop);
	void PlayGenerated(void *func, int sample_rate);
	void Pause();
	void Update();

	bool IsPlaying();
	bool IsPaused();
	AudioFile *GetAudio(){	return audio;	}
	Range GetRange(){	return range;	}
	int GetPos();

	float GetSampleRate();
	BufferBox GetSomeSamples(int num_samples);

	float GetVolume();
	void SetVolume(float _volume);

private:
	bool TestError(const string &msg);
	bool TestError2(const string &msg, void *d);
	bool stream(int buf);


	float volume;
	bool playing;
	bool loop;
	int pos;
	int sample_rate;

	AudioFile *audio;
	Range range;

	typedef void generate_func_t(int, BufferBox &);
	generate_func_t *generate_func;

	Array<short> data;
	int data_samples;

	void *al_context;
	void *al_dev;
	int buffer[2];
	unsigned int source;
	int stream_pos;
	int stream_size;
	int stream_pos_0;

	bool al_initialized;
	int al_last_error;
};

#endif /* AUDIOOUTPUT_H_ */
