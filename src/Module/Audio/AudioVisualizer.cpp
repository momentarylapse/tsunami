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

AudioVisualizer::Output::Output(AudioVisualizer *v) : AudioPort("out")
{
	visualizer = v;
}

int AudioVisualizer::Output::read(AudioBuffer& buf)
{
	if (!visualizer->source)
		return 0;
	int r = visualizer->source->read(buf);
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

int AudioVisualizer::Output::get_pos(int delta)
{
	if (!visualizer->source)
		return 0;
	return visualizer->source->get_pos(delta);
}

void AudioVisualizer::Output::reset()
{
	if (visualizer->source)
		visualizer->source->reset();
	visualizer->reset();
}

AudioVisualizer::AudioVisualizer() :
	Module(ModuleType::AUDIO_VISUALIZER)
{
	out = new Output(this);
	port_out.add(out);
	port_in.add(InPortDescription(SignalType::AUDIO, (Port**)&source, "in"));
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

void AudioVisualizer::set_source(AudioPort *s)
{
	source = s;
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


