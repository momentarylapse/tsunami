/*
 * AudioSucker.h
 *
 *  Created on: 17.09.2017
 *      Author: michi
 */

#ifndef SRC_AUDIO_AUDIOSUCKER_H_
#define SRC_AUDIO_AUDIOSUCKER_H_

#include "AudioBuffer.h"
#include "../Module/Module.h"

class AudioPort;
class AudioSuckerThread;

class AudioSucker : public Module
{
	friend class AudioSuckerThread;
public:
	AudioSucker(Session *s);
	virtual ~AudioSucker();

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

	int perf_channel;

	AudioSuckerThread *thread;
};

#endif /* SRC_AUDIO_AUDIOSUCKER_H_ */
