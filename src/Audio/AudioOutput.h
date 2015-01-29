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

class AudioStream;

class AudioOutput : public Observable
{
public:
	friend class AudioStream;

	AudioOutput();
	virtual ~AudioOutput();

	Array<string> devices;
	string chosen_device;
	void setDevice(const string &device);

	void init();
	void kill();


	float getVolume();
	void setVolume(float _volume);

	void addStream(AudioStream *s);
	void removeStream(AudioStream *s);
	bool streamExists(AudioStream *s);

private:
	bool testError(const string &msg);

	bool initialized;
	int last_error;

	int pa_device_no;

	float volume;

	Array<AudioStream*> streams;
};

#endif /* AUDIOOUTPUT_H_ */
