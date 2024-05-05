/*
 * AudioVisualizer.cpp
 *
 *  Created on: 04.04.2018
 *      Author: michi
 */

#include "AudioVisualizer.h"
#include "../ModuleFactory.h"
#include "../../Session.h"
#include "../../data/audio/AudioBuffer.h"
#include "../../data/audio/RingBuffer.h"
#include "../../data/base.h"
#include "../../stuff/PerformanceMonitor.h"
#include "../../lib/hui/Callback.h"
#include "../../lib/hui/Application.h"

int AudioVisualizer::read_audio(int port, AudioBuffer& buf) {
	if (!in.source)
		return NO_SOURCE;
	int r = in.source->read_audio(buf);
	if (r <= 0)
		return r;

	PerformanceMonitor::start_busy(perf_channel);
	buffer->buf.set_channels(buf.channels);
	buffer->write(buf);

	if (buffer->available() < chunk_size)
		return r;

	lock();
	while (buffer->available() >= chunk_size) {
		AudioBuffer b;
		buffer->read_ref(b, chunk_size);
		process(b);
		buffer->read_ref_done(b);
	}
	unlock();
	PerformanceMonitor::end_busy(perf_channel);

	notify_counter ++;
	hui::run_later(0.001f, [this] {
		out_changed.notify();
		notify_counter --;
	});
	return r;
}

AudioVisualizer::AudioVisualizer() :
	Module(ModuleCategory::AUDIO_VISUALIZER, "")
{
	buffer = new RingBuffer(1 << 18);
	chunk_size = 2084;
	notify_counter = 0;
}

AudioVisualizer::~AudioVisualizer() {
	// make sure all notifications have been handled
	while (notify_counter > 0) {
		hui::Application::do_single_main_loop();
	}
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


