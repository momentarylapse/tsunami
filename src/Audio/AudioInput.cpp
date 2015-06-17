/*
 * AudioInput.cpp
 *
 *  Created on: 22.02.2013
 *      Author: michi
 */

#include "AudioInput.h"
#include "AudioInputAudio.h"
#include "AudioInputMidi.h"
#include "../Tsunami.h"
#include "../Stuff/Log.h"

#define UPDATE_TIME		0.005f

const string AudioInput::MESSAGE_CAPTURE = "Capture";

AudioInput::AudioInput(int _sample_rate) :
	PeakMeterSource("AudioInput"),
	current_buffer(1048576)
{
	sample_rate = _sample_rate;
	running = false;
	type = -1;
	accumulating = false;
	capturing = false;
	hui_runner_id = -1;
}

AudioInput::~AudioInput()
{
	msg_write("~AudioInput");
}

void AudioInput::accumulate(bool enable)
{
	resetAccumulation();
	accumulating = enable;
}

void AudioInput::_startUpdate()
{
	if (running)
		return;
	hui_runner_id = HuiRunRepeatedM(UPDATE_TIME, this, &AudioInput::update);
	running = true;
}

void AudioInput::_stopUpdate()
{
	if (!running)
		return;
	HuiCancelRunner(hui_runner_id);
	hui_runner_id = -1;
	running = false;
}

void AudioInput::update()
{
	if (doCapturing() > 0)
		notify(MESSAGE_CAPTURE);

	running = isCapturing();
}

bool AudioInput::isCapturing()
{
	return capturing;
}

int AudioInput::getState()
{
	if (isCapturing())
		return STATE_PLAYING;
	return STATE_STOPPED;
}
