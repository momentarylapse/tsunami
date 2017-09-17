/*
 * AudioSucker.h
 *
 *  Created on: 17.09.2017
 *      Author: michi
 */

#ifndef SRC_AUDIO_AUDIOSUCKER_H_
#define SRC_AUDIO_AUDIOSUCKER_H_

#include "../Data/AudioBuffer.h"
#include "../Stuff/Observable.h"

class AudioSource;
class AudioSuckerThread;

class AudioSucker : public Observable<VirtualBase>
{
	friend class AudioSuckerThread;
public:
	AudioSucker(AudioSource *source);
	virtual ~AudioSucker();

	void setSource(AudioSource *s);
	void setAccumulate(bool enable);
	void setBufferSize(int size);

	void start();
	void stop();

	int update();
	void wait();

	static const int DEFAULT_BUFFER_SIZE;
	static const string MESSAGE_UPDATE;

	AudioSource *source;
	AudioBuffer buf;
	bool accumulate;
	bool running;
	int buffer_size;
	float no_data_wait;

	float cpu_usage;

	AudioSuckerThread *thread;
};

#endif /* SRC_AUDIO_AUDIOSUCKER_H_ */
