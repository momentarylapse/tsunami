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


#ifdef NIX_OS_WINDOWS
	#include <al.h>
	#include <alut.h>
	#include <alc.h>
	#pragma comment(lib,"alut.lib")
	#pragma comment(lib,"OpenAL32.lib")
	/*#pragma comment(lib,"libogg.lib")
	#pragma comment(lib,"libvorbis.lib")
	#pragma comment(lib,"libvorbisfile.lib")*/

#else
	#include <portaudio.h>
#endif



AudioOutput::AudioOutput() :
	Observable("AudioOutput")
{
	initialized = false;
	last_error = paNoError;

	al_context = NULL;
	al_dev = NULL;

	ChosenDevice = HuiConfig.getStr("Output.ChosenDevice", "");
	volume = HuiConfig.getFloat("Output.Volume", 1.0f);

	init();
}

AudioOutput::~AudioOutput()
{
	kill();
	HuiConfig.setStr("Output.ChosenDevice", ChosenDevice);
	HuiConfig.setFloat("Output.Volume", volume);
}

void AudioOutput::setDevice(const string &device)
{
	ChosenDevice = device;
	HuiConfig.setStr("ChosenOutputDevice", ChosenDevice);
	HuiConfig.save();
	tsunami->log->warning(_("Das neue Ger&at wird erst beim n&achsten Start verwendet!"));
	//KillPreview();
	//PreviewInit();
}

void AudioOutput::init()
{
	if (initialized)
		return;
	msg_db_f("Output.init", 1);

	// which device to use?
	/*string dev_name;
	const char *s = alcGetString(NULL, ALC_DEVICE_SPECIFIER);
	Device.clear();
	while(*s != 0){
		Device.add(string(s));
		if (string(s) == ChosenDevice)
			dev_name = s;
		s += strlen(s) + 1;
	}
	if (dev_name.num == 0)
		ChosenDevice = "";

	// try to open manually
	bool ok = false;
	if (dev_name.num > 0){
		al_dev = alcOpenDevice(dev_name.c_str());
		if (al_dev){
			testError("alcOpenDevice (init)");
			al_context = alcCreateContext(al_dev, NULL);
			testError("alcCreateContext (init)");
			if (al_context){
				if (alcMakeContextCurrent(al_context)){
					tsunami->log->info(_("benutze OpenAl Device: ") + dev_name);
					ok = true;
				}
				testError("alcMakeContextCurrent (init)");
			}
		}
	}

	// failed -> use automatic method
	if (!ok){
		ok = alutInit(NULL, 0);
		testError("alutInit (init)");
		al_dev = NULL;
		al_context = NULL;
	}

	if (!ok){
		tsunami->log->error(string("OpenAL init: ") + alutGetErrorString(last_error));
		return;
	}

	//SetListenerValues();
	testError("init...");*/
	 Pa_Initialize();

	initialized = true;
}

void AudioOutput::kill()
{
	if (!initialized)
		return;
	msg_db_f("Output.kill",1);

	foreach(AudioStream *s, streams)
		s->kill();

	// close devices
	/*if (al_dev){
		// manually
		//msg_write("current context...");
		alcMakeContextCurrent(NULL);
		testError("alcMakeContextCurrent (kill)");
		//msg_write("destroy context...");
		alcDestroyContext(al_context);
		testError("alcDestroyContext (kill)");
		//msg_write("close device...");
		if (!alcCloseDevice(al_dev))
			testError("alcCloseDevice (kill)");
		//msg_write("ok");
	}else{
		// automatically
		int i = alutExit();
		if (i == 0)
			msg_error((char*)alutGetErrorString(alutGetError()));
	}*/
	last_error = Pa_Terminate();
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

bool AudioOutput::testError(const string &msg)
{
	if (last_error != paNoError){
		tsunami->log->error(string("PortAudio error: ") + Pa_GetErrorText(last_error));
		return true;
	}

	/*int error;
	if (al_dev)
		error = alcGetError(al_dev);
	else
		error = alGetError();
	if (error != AL_NO_ERROR){
		last_error = error;
		//tsunami->log->Error();
		msg_error("OpenAL operation: " + msg);
		msg_write(ALError(error));
		//msg_write(alutGetErrorString(error));
		return true;
	}*/
	return false;
}


