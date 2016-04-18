/*
 * AudioInputAny.cpp
 *
 *  Created on: 16.08.2015
 *      Author: michi
 */

#include "AudioInputAny.h"
#include "../Data/Track.h"
#include "AudioInputAudio.h"


const string AudioInputAny::MESSAGE_CAPTURE = "Capture";

AudioInputAny::AudioInputAny(int _sample_rate) :
	PeakMeterSource("AudioInputAny"),
	Observer("AudioInputAny")
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

AudioInputAny::~AudioInputAny()
{
	setType(-1);
}

void AudioInputAny::setType(int _type)
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
		input_audio = new AudioInputAudio(sample_rate);
		buffer = &input_audio->buffer;
		current_buffer = &input_audio->current_buffer;
		input_audio->setSaveMode(save_mode);
		input_audio->setChunkSize(chunk_size);
		input_audio->setUpdateDt(update_dt);
		subscribe(input_audio);
	}
	if (type == Track::TYPE_MIDI){
		input_midi = new AudioInputMidi(sample_rate);
		input_midi->setPreviewSynthesizer(preview_synth);
		midi = &input_midi->midi;
		current_midi = &input_midi->current_midi;
		subscribe(input_midi);
	}
}

bool AudioInputAny::start()
{
	if (type == Track::TYPE_AUDIO)
		return input_audio->start();
	else if (type == Track::TYPE_MIDI)
		return input_midi->start();
	return false;
}

void AudioInputAny::stop()
{
	if (type == Track::TYPE_AUDIO)
		input_audio->stop();
	else if (type == Track::TYPE_MIDI)
		input_midi->stop();
}

bool AudioInputAny::isCapturing()
{
	if (type == Track::TYPE_AUDIO)
		return input_audio->isCapturing();
	else if (type == Track::TYPE_MIDI)
		return input_midi->isCapturing();
	return false;
}

void AudioInputAny::accumulate(bool enable)
{
	if (type == Track::TYPE_AUDIO)
		input_audio->accumulate(enable);
	else if (type == Track::TYPE_MIDI)
		input_midi->accumulate(enable);
}

void AudioInputAny::resetAccumulation()
{
	if (type == Track::TYPE_AUDIO)
		input_audio->resetAccumulation();
	else if (type == Track::TYPE_MIDI)
		input_midi->resetAccumulation();
}

int AudioInputAny::getSampleCount()
{
	if (type == Track::TYPE_AUDIO)
		return input_audio->getSampleCount();
	else if (type == Track::TYPE_MIDI)
		return input_midi->getSampleCount();
	return 0;
}

void AudioInputAny::getSomeSamples(BufferBox& buf, int num_samples)
{
	if (type == Track::TYPE_AUDIO)
		input_audio->getSomeSamples(buf, num_samples);
	else if (type == Track::TYPE_MIDI)
		input_midi->getSomeSamples(buf, num_samples);
}

int AudioInputAny::getState()
{
	if (type == Track::TYPE_AUDIO)
		return input_audio->getState();
	else if (type == Track::TYPE_MIDI)
		return input_midi->getState();
	return -1;
}

void AudioInputAny::setDevice(Device *dev)
{
	if (type == Track::TYPE_AUDIO)
		input_audio->setDevice(dev);
	if (type == Track::TYPE_MIDI)
		input_midi->setDevice(dev);
}

Device *AudioInputAny::getDevice()
{
	if (type == Track::TYPE_AUDIO)
		return input_audio->getDevice();
	if (type == Track::TYPE_MIDI)
		return input_midi->getDevice();
	return NULL;
}

void AudioInputAny::setSaveMode(bool enabled)
{
	save_mode = enabled;
	if (type == Track::TYPE_AUDIO)
		input_audio->setSaveMode(enabled);
}

AudioInputMidi::MidiPort AudioInputAny::getCurMidiPort()
{
	if (type == Track::TYPE_MIDI)
		return input_midi->getCurMidiPort();
	AudioInputMidi::MidiPort p;
	return p;
}

Array<AudioInputMidi::MidiPort> AudioInputAny::findMidiPorts()
{
	if (type == Track::TYPE_MIDI)
		return input_midi->findMidiPorts();
	Array<AudioInputMidi::MidiPort> a;
	return a;
}

bool AudioInputAny::connectMidiPort(AudioInputMidi::MidiPort& p)
{
	if (type == Track::TYPE_MIDI)
		return input_midi->connectMidiPort(p);
	return false;
}

void AudioInputAny::setPreviewSynthesizer(Synthesizer* s)
{
	preview_synth = s;
	if (type == Track::TYPE_MIDI)
		input_midi->setPreviewSynthesizer(preview_synth);
}

void AudioInputAny::setChunkSize(int size)
{
	chunk_size = size;
	if (type == Track::TYPE_AUDIO)
		input_audio->setChunkSize(size);
	else if (type == Track::TYPE_MIDI)
		input_midi->setChunkSize(size);
}

void AudioInputAny::setUpdateDt(float dt)
{
	update_dt = dt;
	if (type == Track::TYPE_AUDIO)
		input_audio->setUpdateDt(dt);
	else if (type == Track::TYPE_MIDI)
		input_midi->setUpdateDt(dt);
}

void AudioInputAny::onUpdate(Observable *o, const string &message)
{
	// just forward the message
	notify(MESSAGE_CAPTURE);
}
