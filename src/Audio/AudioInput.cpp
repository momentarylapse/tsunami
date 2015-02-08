/*
 * AudioInput.cpp
 *
 *  Created on: 22.02.2013
 *      Author: michi
 */

#include "AudioInput.h"
#include "AudioInputBase.h"
#include "AudioInputAudio.h"
#include "AudioInputMidi.h"
#include "../Tsunami.h"
#include "../Stuff/Log.h"

#define UPDATE_TIME		0.005f

const string AudioInput::MESSAGE_CAPTURE = "Capture";

AudioInput::AudioInput() :
	PeakMeterSource("AudioInput"),
	current_buffer(1048576)
{
	in_audio = new AudioInputAudio(buffer, current_buffer);
	in_midi = new AudioInputMidi(midi, current_midi);
	in_cur = in_audio;
	running = false;
	type = -1;
}

AudioInput::~AudioInput()
{
	delete(in_audio);
	delete(in_midi);
}

bool AudioInput::start(int type, int sample_rate)
{
	in_cur->stop();

	if (type == Track::TYPE_AUDIO){
		in_cur = in_audio;
	}else if (type == Track::TYPE_MIDI){
		in_cur = in_midi;
	}else{
		in_cur = in_audio;
		tsunami->log->error(_("Falscher Aufnahme-Typ! (nur AUDIO/MIDI erlaube)"));
		return false;
	}

	if (in_cur->start(sample_rate)){

		if (!running)
			HuiRunLaterM(UPDATE_TIME, this, &AudioInput::update);
		running = true;
		return true;
	}
	tsunami->log->error(_("Konnte Aufnahmeger&at nicht &offnen"));
	return false;
}

void AudioInput::stop()
{
	in_cur->stop();
}


void AudioInput::update()
{
	if (in_cur->doCapturing() > 0)
		notify(MESSAGE_CAPTURE);

	running = in_cur->isCapturing();
	if (running)
		HuiRunLaterM(UPDATE_TIME, this, &AudioInput::update);
}

bool AudioInput::isCapturing()
{
	return in_cur->isCapturing();
}



float AudioInput::getSampleRate()
{
	return in_cur->getSampleRate();
}

void AudioInput::accumulate(bool enable)
{
	in_cur->accumulate(enable);
}

void AudioInput::resetAccumulation()
{
	in_cur->resetAccumulation();
}

int AudioInput::getSampleCount()
{
	return in_cur->getSampleCount();
}

void AudioInput::getSomeSamples(BufferBox &buf, int num_samples)
{
	in_cur->getSomeSamples(buf, num_samples);
}

int AudioInput::getState()
{
	if (isCapturing())
		return STATE_PLAYING;
	return STATE_STOPPED;
}

void AudioInput::resetSync()
{
	in_cur->resetSync();
}

int AudioInput::getDelay()
{
	return in_cur->getDelay();
}
