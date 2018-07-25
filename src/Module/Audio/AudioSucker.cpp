/*
 * AudioSucker.cpp
 *
 *  Created on: 17.09.2017
 *      Author: michi
 */

#include "AudioSucker.h"
#include "../../lib/threads/Thread.h"
#include "../../lib/hui/hui.h"
#include "../../Stuff/PerformanceMonitor.h"
#include "../Port/AudioPort.h"
#include "../ModuleFactory.h"
#include "../../Data/base.h"

const int AudioSucker::DEFAULT_BUFFER_SIZE = 1024;
const string AudioSucker::MESSAGE_UPDATE = "Update";


class AudioSuckerThread : public Thread
{
public:
	AudioSucker *sucker;
	int perf_channel;

	AudioSuckerThread(AudioSucker *s)
	{
		perf_channel = PerformanceMonitor::create_channel("suck");
		sucker = s;
	}
	~AudioSuckerThread()
	{
		PerformanceMonitor::delete_channel(perf_channel);
	}

	void on_run() override
	{
		//msg_write("thread run");
		while(true){
			//msg_write(".");
			if (sucker->running){
				PerformanceMonitor::start_busy(perf_channel);
				int r = sucker->update();
				PerformanceMonitor::end_busy(perf_channel);
				if (r == AudioPort::END_OF_STREAM)
					break;
				if (r == AudioPort::NOT_ENOUGH_DATA){
					hui::Sleep(sucker->no_data_wait);
					continue;
				}
			}else{
				hui::Sleep(0.200f);
			}
			Thread::cancelation_point();
		}
		//msg_write("thread done...");
	}
};

AudioSucker::AudioSucker() :
	Module(ModuleType::AUDIO_SUCKER)
{
	port_in.add(PortDescription(SignalType::AUDIO, (Port**)&source, "in"));
	source = NULL;
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
}

void AudioSucker::set_source(AudioPort* s)
{
	source = s;
}

void AudioSucker::accumulate(bool enable)
{
	accumulating = enable;
}

void AudioSucker::reset_accumulation()
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
	if (r == source->NOT_ENOUGH_DATA)
		return r;
	if (r == source->END_OF_STREAM)
		return r;
	if (accumulating){
		temp.resize(r);
		buf.append(temp);
	}
	Observable::notify(MESSAGE_UPDATE);
	return r;
}


AudioSucker *CreateAudioSucker(Session *session)
{
	return (AudioSucker*)ModuleFactory::create(session, ModuleType::AUDIO_SUCKER, "");
}
