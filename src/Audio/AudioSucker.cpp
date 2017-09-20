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
#include "../Stuff/PerformanceMonitor.h"

const int AudioSucker::DEFAULT_BUFFER_SIZE = 1024;
const string AudioSucker::MESSAGE_UPDATE = "Update";


class AudioSuckerThread : public Thread
{
public:
	AudioSucker *sucker;
	int perf_channel;

	AudioSuckerThread(AudioSucker *s)
	{
		sucker = s;
		perf_channel = sucker->perf_channel;
	}

	virtual void _cdecl onRun()
	{
		//msg_write("thread run");
		while(true){
			//msg_write(".");
			if (sucker->running){
				PerformanceMonitor::start_busy(perf_channel);
				int r = sucker->update();
				PerformanceMonitor::end_busy(perf_channel);
				if (r == AudioSource::END_OF_STREAM)
					break;
				if (r == 0){
					hui::Sleep(sucker->no_data_wait);
					continue;
				}
			}else{
				hui::Sleep(0.200f);
			}
		}
		//msg_write("thread done...");
	}
};

AudioSucker::AudioSucker(AudioSource *_source)
{
	perf_channel = PerformanceMonitor::create_channel("suck");
	source = _source;
	accumulating = false;
	running = false;
	thread = new AudioSuckerThread(this);
	buffer_size = DEFAULT_BUFFER_SIZE;
	no_data_wait = 0.005f;

}

AudioSucker::~AudioSucker()
{
	if (thread){
		thread->kill();
		delete(thread);
		thread = NULL;
	}
	PerformanceMonitor::delete_channel(perf_channel);
}

void AudioSucker::setSource(AudioSource* s)
{
	source = s;
}

void AudioSucker::accumulate(bool enable)
{
	accumulating = enable;
}

void AudioSucker::resetAccumulation()
{
	buf.clear();
}

void AudioSucker::start()
{
	if (running)
		return;
	thread->run();
	running = true;
}

void AudioSucker::stop()
{
	thread->kill();
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
		if (accumulating){
			temp.resize(r);
			buf.append(temp);
		}
		notify(MESSAGE_UPDATE);
	}
	return r;
}