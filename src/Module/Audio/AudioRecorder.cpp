/*
 * AudioRecorder.cpp
 *
 *  Created on: 07.03.2019
 *      Author: michi
 */

#include "AudioRecorder.h"
#include "../Port/Port.h"
#include "../../Data/base.h"


int AudioRecorder::Output::read_audio(AudioBuffer& buf)
{
	if (!rec->source)
		return buf.length;

	int r = rec->source->read_audio(buf);

	if (rec->accumulating and (r > 0)){
		rec->buf.append(buf.ref(0, r));
	}

	return r;
}

AudioRecorder::Output::Output(AudioRecorder *r) : Port(SignalType::AUDIO, "out")
{
	rec = r;
}

AudioRecorder::AudioRecorder() :
	Module(ModuleType::PLUMBING, "AudioRecorder")
{
	port_out.add(new Output(this));
	port_in.add(InPortDescription(SignalType::AUDIO, &source, "in"));
	source = nullptr;
	accumulating = false;
}

void AudioRecorder::accumulate(bool enable)
{
	accumulating = enable;
}

void AudioRecorder::reset_state()
{
	buf.clear();
}

void AudioRecorder::command(ModuleCommand cmd)
{
	if (cmd == ModuleCommand::ACCUMULATION_START)
		accumulate(true);
	else if (cmd == ModuleCommand::ACCUMULATION_STOP)
		accumulate(false);
}
