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

#define UPDATE_TIME		5

AudioInput::AudioInput() :
	PeakMeterSource("AudioInput")
{
	in_audio = new AudioInputAudio(current_buffer);
	in_midi = new AudioInputMidi(midi);
	in_cur = in_audio;
}

AudioInput::~AudioInput()
{
	delete(in_audio);
	delete(in_midi);
}

bool AudioInput::Start(int type, int sample_rate)
{
	in_cur->Stop();

	if (type == Track::TYPE_AUDIO)
		in_cur = in_audio;
	else if (type == Track::TYPE_MIDI)
		in_cur = in_midi;
	else{
		in_cur = in_audio;
		tsunami->log->Error(_("Falscher Aufnahme-Typ! (nur AUDIO/MIDI erlaube)"));
		return false;
	}

	if (in_cur->Start(sample_rate)){

		HuiRunLaterM(UPDATE_TIME, this, &AudioInput::Update);
		return true;
	}
	tsunami->log->Error(_("Konnte Aufnahmeger&at nicht &offnen"));
	return false;
}

void AudioInput::Stop()
{
	in_cur->Stop();
}


void AudioInput::Update()
{
	if (in_cur->DoCapturing() > 0)
		Notify("Capture");

	if (in_cur->IsCapturing())
		HuiRunLaterM(UPDATE_TIME, this, &AudioInput::Update);
}

bool AudioInput::IsCapturing()
{
	return in_cur->IsCapturing();
}



float AudioInput::GetSampleRate()
{
	return in_cur->GetSampleRate();
}

BufferBox AudioInput::GetSomeSamples(int num_samples)
{
	return in_cur->GetSomeSamples(num_samples);
}

void AudioInput::ResetSync()
{
	in_cur->ResetSync();
}

int AudioInput::GetDelay()
{
	return in_cur->GetDelay();
}
