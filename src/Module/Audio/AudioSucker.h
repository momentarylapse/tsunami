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

class AudioPort;
class AudioSuckerThread;

class AudioSucker : public Module
{
	friend class AudioSuckerThread;
public:
	AudioSucker();
	~AudioSucker() override;

	void set_source(AudioPort *s);
	void accumulate(bool enable);
	void reset_accumulation();
	void set_buffer_size(int size);

	void start();
	void stop();

	int update();
	void wait();

	static const int DEFAULT_BUFFER_SIZE;
	static const string MESSAGE_UPDATE;

	AudioPort *source;
	AudioBuffer buf;
	bool accumulating;
	bool running;
	int buffer_size;
	float no_data_wait;

	AudioSuckerThread *thread;
	void module_start() override { start(); }
	void module_stop() override { stop(); }
};

AudioSucker *CreateAudioSucker(Session *session);

#endif /* SRC_MODULE_AUDIO_AUDIOSUCKER_H_ */
