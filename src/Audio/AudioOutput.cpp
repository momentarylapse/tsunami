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

#include <pulse/pulseaudio.h>



AudioOutput::AudioOutput() :
	Observable("AudioOutput")
{
	initialized = false;

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
	HuiConfig.setStr("Output.ChosenDevice", device);

	/*pa_device_no = -1;

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

	tsunami->log->info(format("output device '%s' chosen", Pa_GetDeviceInfo(pa_device_no)->name));*/
}

Array<string> AudioOutput::getDevices()
{
	Array<string> devices;
	msg_write("------get dev");

	/*int n = Pa_GetDeviceCount();
	for (int i=0; i<n; i++){
		const PaDeviceInfo *di = Pa_GetDeviceInfo(i);
		msg_write(di->name);
		msg_write(di->hostApi);
		msg_write(di->maxOutputChannels);
		msg_write(di->maxInputChannels);
		if (di->maxOutputChannels >= 2)
			devices.add(di->name);
	}*/

	return devices;
}

void AudioOutput::init()
{
	if (initialized)
		return;
	msg_db_f("Output.init", 1);



	pa_threaded_mainloop* m = pa_threaded_mainloop_new();
	pa_mainloop_api *mainloop_api = pa_threaded_mainloop_get_api(m);
	context = pa_context_new(mainloop_api, "tsunami");
	if (!context){
		tsunami->log->error("pa_context_new() failed.");
		return;
	}

	int pa_ready = 0;
	//pa_context_set_state_callback(context, pa_state_cb, &pa_ready);

	if (pa_context_connect(context, NULL, (pa_context_flags_t)0, NULL) < 0) {
		tsunami->log->error(string("pa_context_connect() failed: ") + pa_strerror(pa_context_errno(context)));
		return;
	}
	pa_threaded_mainloop_start(m);

	pa_operation *pa_op;

	printf("init...\n");
	while (pa_context_get_state(context) != PA_CONTEXT_READY)
        	{}//pa_mainloop_iterate(m, 1, NULL);
	printf("ok\n");

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

	// TODO

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
	return false;
}


