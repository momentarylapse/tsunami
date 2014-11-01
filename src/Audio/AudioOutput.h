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

struct ALCdevice_struct;
struct ALCcontext_struct;
class AudioStream;

class AudioOutput : public Observable
{
public:
	friend class AudioStream;

	AudioOutput();
	virtual ~AudioOutput();

	Array<string> Device;
	string ChosenDevice;
	void setDevice(const string &device);

	void init();
	void kill();


	float getVolume();
	void setVolume(float _volume);

private:
	bool testError(const string &msg);

	ALCcontext_struct *al_context;
	ALCdevice_struct *al_dev;

	bool al_initialized;
	int al_last_error;

	float volume;

	Array<AudioStream*> streams;
};

#endif /* AUDIOOUTPUT_H_ */
