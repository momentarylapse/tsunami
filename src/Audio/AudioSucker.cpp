/*
 * AudioSucker.cpp
 *
 *  Created on: 17.09.2017
 *      Author: michi
 */

#include "AudioSucker.h"
#include "Source/AudioSource.h"
#include "../lib/threads/Thread.h"
#include "../lib/hui/hui.h"

const int AudioSucker::DEFAULT_BUFFER_SIZE = 4096;
const string AudioSucker::MESSAGE_UPDATE = "Update";


class AudioSuckerThread : public Thread
{
public:
	AudioSucker *sucker;
	hui::Timer timer;
	float t_idle;

	AudioSuckerThread(AudioSucker *s)
	{
		sucker = s;
		t_idle = 0;
	}

	virtual void _cdecl onRun()
	{
		timer.reset();
		//msg_write("thread run");
		while(true){
			if (sucker->running){
				int r = sucker->update();
				if (r == AudioSource::END_OF_STREAM)
					break;
				if (r == 0){
					hui::Sleep(sucker->no_data_wait);
					t_idle += timer.get();
					continue;
				}

				float t_busy = timer.get();
				sucker->cpu_usage = t_busy / (t_busy + t_idle);
				printf("%.1f %%\n", sucker->cpu_usage * 100);
				t_idle = 0;
			}else{
				hui::Sleep(0.200f);
				t_idle += timer.get();
			}
		}
		//msg_write("thread done...");
	}
};

AudioSucker::AudioSucker(AudioSource *_source)
{
	source = _source;
	accumulate = false;
	running = false;
	thread = new AudioSuckerThread(this);
	buffer_size = DEFAULT_BUFFER_SIZE;
	cpu_usage = 0;
	no_data_wait = 0.005f;
}

AudioSucker::~AudioSucker()
{
	if (thread){
		thread->kill();
		delete(thread);
		thread = NULL;
	}
}

void AudioSucker::setSource(AudioSource* s)
{
	source = s;
}

void AudioSucker::setAccumulate(bool enable)
{
	accumulate = enable;
}

void AudioSucker::start()
{
	running = true;
}

void AudioSucker::stop()
{
	running = false;
}

int AudioSucker::update()
{
	AudioBuffer temp;
	temp.resize(buffer_size);
	int r = source->read(temp);
	if (r == source->END_OF_STREAM)
		return r;
	if (r > 0){
		if (accumulate)
			buf.append(temp);
		notify(MESSAGE_UPDATE);
	}
	return r;
}
