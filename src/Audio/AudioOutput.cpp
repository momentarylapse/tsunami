/*
 * AudioOutput.cpp
 *
 *  Created on: 26.03.2012
 *      Author: michi
 */

#include "AudioOutput.h"
#include "AudioStream.h"
#include "../Tsunami.h"
#include "../Stuff/Log.h"

#include <portaudio.h>



AudioOutput::AudioOutput() :
	Observable("AudioOutput")
{
	initialized = false;
	last_error = paNoError;

	chosen_device = HuiConfig.getStr("Output.ChosenDevice", "");
	volume = HuiConfig.getFloat("Output.Volume", 1.0f);

	init();
}

AudioOutput::~AudioOutput()
{
	kill();
	HuiConfig.setStr("Output.ChosenDevice", chosen_device);
	HuiConfig.setFloat("Output.Volume", volume);
}

void AudioOutput::setDevice(const string &device)
{
	chosen_device = device;
	//HuiConfig.setStr("ChosenOutputDevice", chosen_device);
	//HuiConfig.save();

	pa_device_no = -1;

	int n = Pa_GetDeviceCount();
	for (int i=0; i<n; i++){
		const PaDeviceInfo *di = Pa_GetDeviceInfo(i);
		if (chosen_device == string(di->name))
			pa_device_no = i;
	}

	if (pa_device_no < 0){
		pa_device_no = Pa_GetDefaultOutputDevice();
		if (device != "")
			tsunami->log->error(format("output device '%s' not found. Using default.", device.c_str()));
	}

	tsunami->log->info(format("output device '%s' chosen", Pa_GetDeviceInfo(pa_device_no)->name));
}

void AudioOutput::init()
{
	if (initialized)
		return;
	msg_db_f("Output.init", 1);

	last_error = Pa_Initialize();
	testError("Output.initialize");

	devices.clear();

	int n = Pa_GetDeviceCount();
	for (int i=0; i<n; i++){
		const PaDeviceInfo *di = Pa_GetDeviceInfo(i);
		if (di->maxOutputChannels >= 2)
			devices.add(di->name);
	}

	setDevice(chosen_device);

	initialized = true;
}

void AudioOutput::kill()
{
	if (!initialized)
		return;
	msg_db_f("Output.kill",1);

	foreach(AudioStream *s, streams)
		s->kill();

	last_error = Pa_Terminate();
	testError("terminate");
	initialized = false;
}


float AudioOutput::getVolume()
{
	return volume;
}

void AudioOutput::setVolume(float _volume)
{
	volume = _volume;
	notify(MESSAGE_CHANGE);
}

void AudioOutput::addStream(AudioStream* s)
{
	streams.add(s);
}

void AudioOutput::removeStream(AudioStream* s)
{
	for (int i=streams.num-1; i>=0; i--)
		if (streams[i] == s)
			streams.erase(i);
}

bool AudioOutput::streamExists(AudioStream* s)
{
	for (int i=streams.num-1; i>=0; i--)
		if (streams[i] == s)
			return true;
	return false;
}

bool AudioOutput::testError(const string &msg)
{
	if (last_error != paNoError){
		tsunami->log->error(string("PortAudio (general output) error: ") + Pa_GetErrorText(last_error));
		return true;
	}
	return false;
}


