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

namespace tsunami {

int AudioVisualizer::read_audio(int port, AudioBuffer& buf) {
	if (!in.source)
		return Return::NoSource;
	int r = in.source->read_audio(buf);
	if (r <= 0)
		return r;

	feed(buf);

	return r;
}

void AudioVisualizer::feed(const AudioBuffer& buf) {
	buffer->buf.set_channels(buf.channels);
	buffer->write(buf);
}

AudioVisualizer::AudioVisualizer() :
	Module(ModuleCategory::AudioVisualizer, "")
{
	buffer = new RingBuffer(1 << 18);
	chunk_size = 2084;

	id_runner = hui::run_repeated(0.02f, [this] {

		if (buffer->available() < chunk_size)
			return;

		PerformanceMonitor::start_busy(perf_channel);
		while (buffer->available() >= chunk_size) {
			AudioBuffer b;
			buffer->read_ref(b, chunk_size);
			process(b);
			buffer->read_ref_done(b);
			out_changed.notify();
		}
		PerformanceMonitor::end_busy(perf_channel);
	});
}

AudioVisualizer::~AudioVisualizer() {
	hui::cancel_runner(id_runner);
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

}


