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

AudioVisualizer::Output::Output(AudioVisualizer *v) : Port(SignalType::AUDIO, "out")
{
	visualizer = v;
}

int AudioVisualizer::Output::read_audio(AudioBuffer& buf)
{
	if (!visualizer->source)
		return 0;
	int r = visualizer->source->read_audio(buf);
	if (r <= 0)
		return r;

	visualizer->buffer->write(buf);

	while (visualizer->buffer->available() >= visualizer->chunk_size){
		AudioBuffer b;
		visualizer->buffer->read_ref(b, visualizer->chunk_size);
		visualizer->process(b);
		visualizer->buffer->read_ref_done(b);
	}
	return r;
}

AudioVisualizer::AudioVisualizer() :
	Module(ModuleType::AUDIO_VISUALIZER, "")
{
	port_out.add(new Output(this));
	port_in.add(InPortDescription(SignalType::AUDIO, &source, "in"));
	source = nullptr;
	buffer = new RingBuffer(1 << 18);
	chunk_size = 2084;
}

AudioVisualizer::~AudioVisualizer()
{
	delete buffer;
}

void AudioVisualizer::__init__()
{
	new(this) AudioVisualizer;
}

void AudioVisualizer::__delete__()
{
	this->AudioVisualizer::~AudioVisualizer();
}

void AudioVisualizer::set_chunk_size(int _chunk_size)
{
	chunk_size = _chunk_size;
}

// TODO: move to PluginManager?
AudioVisualizer *CreateAudioVisualizer(Session *session, const string &name)
{
	return (AudioVisualizer*)ModuleFactory::create(session, ModuleType::AUDIO_VISUALIZER, name);
}


