/*
 * AudioOutput.h
 *
 *  Created on: 26.03.2012
 *      Author: michi
 */

#ifndef AUDIOOUTPUT_H_
#define AUDIOOUTPUT_H_

#include "../lib/file/file.h"
#include "../lib/hui/hui.h"
#include "../Data/AudioFile.h"
#include "../Stuff/Observable.h"

class AudioOutput : public HuiEventHandler, public Observable
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
	void Update();

	bool IsPlaying();
	int GetPos(AudioFile * a);
	void GetPeaks(float &peak_r, float &peak_l);

	float GetVolume();
	void SetVolume(float _volume);

private:
	bool TestError(const string &msg);
	bool TestError2(const string &msg, void *d);
	bool stream(int buf);


	float volume;
	bool playing;
	bool loop;
	int start, pos;

	AudioFile *audio;

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
