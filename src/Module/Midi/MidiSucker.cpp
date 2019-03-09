/*
 * MidiSucker.cpp
 *
 *  Created on: 09.03.2019
 *      Author: michi
 */

#include "MidiSucker.h"

#include "../../lib/threads/Thread.h"
#include "../../lib/hui/hui.h"
#include "../../Stuff/PerformanceMonitor.h"
#include "../Port/Port.h"
#include "../ModuleFactory.h"
#include "../../Data/base.h"
#include "../../Data/Midi/MidiData.h"

const int MidiSucker::DEFAULT_BUFFER_SIZE = 1024;


class MidiSuckerThread : public Thread
{
public:
	MidiSucker *sucker;
	int perf_channel;
	bool keep_running = true;

	MidiSuckerThread(MidiSucker *s)
	{
		perf_channel = PerformanceMonitor::create_channel("suck");
		sucker = s;
	}
	~MidiSuckerThread()
	{
		PerformanceMonitor::delete_channel(perf_channel);
	}

	void on_run() override
	{
		//msg_write("thread run");
		while(keep_running){
			//msg_write(".");
			if (sucker->running){
				PerformanceMonitor::start_busy(perf_channel);
				int r = sucker->update();
				PerformanceMonitor::end_busy(perf_channel);
				if (r == Port::END_OF_STREAM)
					break;
				if (r == Port::NOT_ENOUGH_DATA){
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

MidiSucker::MidiSucker() :
	Module(ModuleType::PLUMBING, "MidiSucker")
{
	port_in.add(InPortDescription(SignalType::MIDI, &source, "in"));
	source = nullptr;
	running = false;
	thread = nullptr;//new MidiSuckerThread(this);
	buffer_size = DEFAULT_BUFFER_SIZE;
	no_data_wait = 0.005f;

}

MidiSucker::~MidiSucker()
{
	if (thread){
		thread->keep_running = false;
		thread->join();
		//thread->kill();
		delete(thread);
		thread = nullptr;
	}
}

void MidiSucker::start()
{
	if (running)
		return;
	thread = new MidiSuckerThread(this);
	thread->run();
	running = true;
}

void MidiSucker::stop()
{
	if (thread){
		thread->keep_running = false;
		thread->join();
		delete thread;
		thread = nullptr;
	}
	running = false;
}

int MidiSucker::command(ModuleCommand cmd, int param)
{
	if (cmd == ModuleCommand::START){
		start();
		return 0;
	}else if (cmd == ModuleCommand::STOP){
		stop();
		return 0;
	}
	return COMMAND_NOT_HANDLED;
}

int MidiSucker::update()
{
	MidiEventBuffer buf;
	buf.samples = buffer_size;
	int r = source->read_midi(buf);
	if (r == source->NOT_ENOUGH_DATA)
		return r;
	if (r == source->END_OF_STREAM)
		return r;
	return r;
}

