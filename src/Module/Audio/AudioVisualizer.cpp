/*
 * AudioVisualizer.cpp
 *
 *  Created on: 04.04.2018
 *      Author: michi
 */

#include "AudioVisualizer.h"
#include "../ModuleFactory.h"
#include "../../Session.h"
#include "../../Data/Audio/AudioBuffer.h"
#include "../../Data/Audio/RingBuffer.h"
#include "../../Data/base.h"
#include "../../Stuff/PerformanceMonitor.h"
#include "../../lib/hui/Callback.h"

AudioVisualizer::Output::Output(AudioVisualizer *v) : Port(SignalType::AUDIO, "out") {
	visualizer = v;
}

int AudioVisualizer::Output::read_audio(AudioBuffer& buf) {
	if (!visualizer->source)
		return NO_SOURCE;
	int r = visualizer->source->read_audio(buf);
	if (r <= 0)
		return r;

	PerformanceMonitor::start_busy(visualizer->perf_channel);
	visualizer->buffer->buf.set_channels(buf.channels);
	visualizer->buffer->write(buf);

	if (visualizer->buffer->available() < visualizer->chunk_size)
		return r;

	visualizer->lock();
	while (visualizer->buffer->available() >= visualizer->chunk_size) {
		AudioBuffer b;
		visualizer->buffer->read_ref(b, visualizer->chunk_size);
		visualizer->process(b);
		visualizer->buffer->read_ref_done(b);
	}
	visualizer->unlock();
	PerformanceMonitor::end_busy(visualizer->perf_channel);

	hui::RunLater(0.001f, [=] {
		visualizer->notify();
	});
	return r;
}

AudioVisualizer::AudioVisualizer() :
	Module(ModuleCategory::AUDIO_VISUALIZER, "")
{
	port_out.add(new Output(this));
	port_in.add(InPortDescription(SignalType::AUDIO, &source, "in"));
	source = nullptr;
	buffer = new RingBuffer(1 << 18);
	chunk_size = 2084;
}

void AudioVisualizer::__init__() {
	new(this) AudioVisualizer;
}

void AudioVisualizer::__delete__() {
	this->AudioVisualizer::~AudioVisualizer();
}

void AudioVisualizer::set_chunk_size(int _chunk_size) {
	chunk_size = _chunk_size;
}

void AudioVisualizer::lock() {

}

void AudioVisualizer::unlock() {

}

void AudioVisualizer::flip() {
	if (next_writing > 0)
		next_writing = 0;
	else
		next_writing = 1;
	current_reading = 1 - next_writing;
}

// TODO: move to PluginManager?
AudioVisualizer *CreateAudioVisualizer(Session *session, const string &name) {
	return (AudioVisualizer*)ModuleFactory::create(session, ModuleCategory::AUDIO_VISUALIZER, name);
}


