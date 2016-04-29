/*
 * InputStreamAny.cpp
 *
 *  Created on: 16.08.2015
 *      Author: michi
 */

#include "../Data/Track.h"
#include "InputStreamAny.h"
#include "InputStreamAudio.h"


const string InputStreamAny::MESSAGE_CAPTURE = "Capture";

InputStreamAny::InputStreamAny(int _sample_rate) :
	PeakMeterSource("InputStreamAny"),
	Observer("InputStreamAny")
{
	sample_rate = _sample_rate;
	chunk_size = -1;
	update_dt = -1;
	type = -1;
	input_audio = NULL;
	input_midi = NULL;
	buffer = NULL;
	current_buffer = NULL;
	midi = NULL;
	current_midi = NULL;
	preview_synth = NULL;
	save_mode = false;
}

InputStreamAny::~InputStreamAny()
{
	setType(-1);
}

void InputStreamAny::setType(int _type)
{
	if (type == _type)
		return;

	if (type == Track::TYPE_AUDIO){
		unsubscribe(input_audio);
		delete(input_audio);
	}
	if (type == Track::TYPE_MIDI){
		unsubscribe(input_midi);
		delete(input_midi);
	}

	type = _type;

	if (type == Track::TYPE_AUDIO){
		input_audio = new InputStreamAudio(sample_rate);
		buffer = &input_audio->buffer;
		current_buffer = &input_audio->current_buffer;
		input_audio->setSaveMode(save_mode);
		input_audio->setChunkSize(chunk_size);
		input_audio->setUpdateDt(update_dt);
		subscribe(input_audio);
	}
	if (type == Track::TYPE_MIDI){
		input_midi = new InputStreamMidi(sample_rate);
		input_midi->setPreviewSynthesizer(preview_synth);
		midi = &input_midi->midi;
		current_midi = &input_midi->current_midi;
		subscribe(input_midi);
	}
}

bool InputStreamAny::start()
{
	if (type == Track::TYPE_AUDIO)
		return input_audio->start();
	else if (type == Track::TYPE_MIDI)
		return input_midi->start();
	return false;
}

void InputStreamAny::stop()
{
	if (type == Track::TYPE_AUDIO)
		input_audio->stop();
	else if (type == Track::TYPE_MIDI)
		input_midi->stop();
}

bool InputStreamAny::isCapturing()
{
	if (type == Track::TYPE_AUDIO)
		return input_audio->isCapturing();
	else if (type == Track::TYPE_MIDI)
		return input_midi->isCapturing();
	return false;
}

void InputStreamAny::accumulate(bool enable)
{
	if (type == Track::TYPE_AUDIO)
		input_audio->accumulate(enable);
	else if (type == Track::TYPE_MIDI)
		input_midi->accumulate(enable);
}

void InputStreamAny::resetAccumulation()
{
	if (type == Track::TYPE_AUDIO)
		input_audio->resetAccumulation();
	else if (type == Track::TYPE_MIDI)
		input_midi->resetAccumulation();
}

int InputStreamAny::getSampleCount()
{
	if (type == Track::TYPE_AUDIO)
		return input_audio->getSampleCount();
	else if (type == Track::TYPE_MIDI)
		return input_midi->getSampleCount();
	return 0;
}

void InputStreamAny::getSomeSamples(BufferBox& buf, int num_samples)
{
	if (type == Track::TYPE_AUDIO)
		input_audio->getSomeSamples(buf, num_samples);
	else if (type == Track::TYPE_MIDI)
		input_midi->getSomeSamples(buf, num_samples);
}

int InputStreamAny::getState()
{
	if (type == Track::TYPE_AUDIO)
		return input_audio->getState();
	else if (type == Track::TYPE_MIDI)
		return input_midi->getState();
	return -1;
}

void InputStreamAny::setDevice(Device *dev)
{
	if (type == Track::TYPE_AUDIO)
		input_audio->setDevice(dev);
	if (type == Track::TYPE_MIDI)
		input_midi->setDevice(dev);
}

Device *InputStreamAny::getDevice()
{
	if (type == Track::TYPE_AUDIO)
		return input_audio->getDevice();
	if (type == Track::TYPE_MIDI)
		return input_midi->getDevice();
	return NULL;
}

void InputStreamAny::setSaveMode(bool enabled)
{
	save_mode = enabled;
	if (type == Track::TYPE_AUDIO)
		input_audio->setSaveMode(enabled);
}


void InputStreamAny::setPreviewSynthesizer(Synthesizer* s)
{
	preview_synth = s;
	if (type == Track::TYPE_MIDI)
		input_midi->setPreviewSynthesizer(preview_synth);
}

void InputStreamAny::setChunkSize(int size)
{
	chunk_size = size;
	if (type == Track::TYPE_AUDIO)
		input_audio->setChunkSize(size);
	else if (type == Track::TYPE_MIDI)
		input_midi->setChunkSize(size);
}

void InputStreamAny::setUpdateDt(float dt)
{
	update_dt = dt;
	if (type == Track::TYPE_AUDIO)
		input_audio->setUpdateDt(dt);
	else if (type == Track::TYPE_MIDI)
		input_midi->setUpdateDt(dt);
}

void InputStreamAny::onUpdate(Observable *o, const string &message)
{
	// just forward the message
	notify(MESSAGE_CAPTURE);
}
