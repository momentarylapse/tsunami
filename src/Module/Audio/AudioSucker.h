/*
 * AudioSucker.h
 *
 *  Created on: 17.09.2017
 *      Author: michi
 */

#ifndef SRC_MODULE_AUDIO_AUDIOSUCKER_H_
#define SRC_MODULE_AUDIO_AUDIOSUCKER_H_

#include "../../Data/Audio/AudioBuffer.h"
#include "../Module.h"

class Port;
class AudioSuckerThread;

class AudioSucker : public Module
{
	friend class AudioSuckerThread;
public:
	AudioSucker();
	~AudioSucker() override;

	void accumulate(bool enable);
	void set_buffer_size(int size);

	void reset_state() override;

	void start();
	void stop();

	int update();

	static const int DEFAULT_BUFFER_SIZE;

	Port *source;
	AudioBuffer buf;
	bool accumulating;
	bool running;
	int buffer_size;
	float no_data_wait;

	AudioSuckerThread *thread;
	void command(ModuleCommand cmd) override;
};

#endif /* SRC_MODULE_AUDIO_AUDIOSUCKER_H_ */
