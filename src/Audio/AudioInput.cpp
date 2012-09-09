/*
 * AudioInput.cpp
 *
 *  Created on: 26.03.2012
 *      Author: michi
 */

#include "AudioInput.h"
#include "../lib/hui/hui.h"
#include "../Tsunami.h"


#define UPDATE_TIME		10


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
	#include <AL/al.h>
	#include <AL/alut.h>
	#include <AL/alc.h>
#endif

AudioInput::AudioInput() :
	PeakMeterSource("AudioInput")
{
	Capturing = false;
	capture = NULL;
	CapturingByDialog = false;
	CaptureAddData = false;
	CapturePlayback = false;
	CapturePlaybackDelay = 0;
	memset(capture_temp, 0, sizeof(capture_temp));
	CaptureSampleRate = DEFAULT_SAMPLE_RATE;
	CaptureMaxDelay = 0;
	CaptureCurrentSamples = 0;

	ChosenDevice = HuiConfigReadStr("Input.ChosenDevice", "");
	CapturePlaybackDelay = HuiConfigReadFloat("Input.PlaybackDelay", 80.0f);

}

AudioInput::~AudioInput()
{
}


void AudioInput::Init()
{
	msg_db_r("CaptureInit", 1);
	Device.clear();
	const ALCchar *s = alcGetString(NULL, ALC_CAPTURE_DEVICE_SPECIFIER);
	while(*s != 0){
		Device.add(string(s));
		if (string(s) == ChosenDevice)
			dev_name = s;
		s += strlen(s) + 1;
	}
	if (dev_name == "")
		ChosenDevice = "";
	msg_db_l(1);
}

void AudioInput::Stop()
{
	msg_db_r("CaptureStop", 1);
	if (Capturing){
		alcCaptureStop((ALCdevice*)capture);
		alcCaptureCloseDevice((ALCdevice*)capture);
		Capturing = false;
		CaptureAddData = false;
	}
	msg_db_l(1);
}

bool AudioInput::Start(int sample_rate, bool add_data)
{
	msg_db_r("CaptureStart", 1);
	if (Capturing)
		Stop();

	Init();
	CaptureSampleRate = sample_rate;
	capture = alcCaptureOpenDevice(dev_name.c_str(), sample_rate, AL_FORMAT_STEREO16, NUM_CAPTURE_SAMPLES);
	//msg_write((int)capture);
	if (capture){
		alcCaptureStart((ALCdevice*)capture);
		Capturing = true;
		CaptureAddData = add_data;
		CaptureMaxDelay = 0;
		HuiRunLaterM(UPDATE_TIME, this, (void(HuiEventHandler::*)())&AudioInput::Update);
	}
	if ((!Capturing) && (CapturingByDialog))
		tsunami->log->Error(_("Konnte Aufnahmeger&at nicht &offnen"));
	msg_db_l(1);
	return Capturing;
}


void AudioInput::AddToCaptureBuf(int a)
{
	int i0 = CaptureBuf.num;
	CaptureBuf.resize(i0 + a);
	CaptureBuf.set_16bit(capture_temp, i0, a);
}

void AudioInput::AddToCapturePreviewBuf(int a)
{
	CapturePreviewBuf.resize(a);
	CapturePreviewBuf.set_16bit(capture_temp, 0, a);
}

int AudioInput::DoCapturing()
{
	msg_db_r("DoCapturing", 1);
	int a = -42;
	alcGetIntegerv((ALCdevice*)capture, ALC_CAPTURE_SAMPLES, 1, &a);

	// don't wait, till we really have as much data as we requested
	//   (or else it might freeze up....)
	if (a >= NUM_CAPTURE_SAMPLES / 8){
		if (CaptureAddData){

			if (a >= NUM_CAPTURE_SAMPLES)
				msg_error("too much capture data!!!");

			// append to buffer
			alcCaptureSamples((ALCdevice*)capture, capture_temp, a);
			AddToCaptureBuf(a);
		}else{

			// fill preview buffer...
			if (a >= NUM_CAPTURE_SAMPLES)
				a = NUM_CAPTURE_SAMPLES;
			alcCaptureSamples((ALCdevice*)capture, capture_temp, a);

			AddToCapturePreviewBuf(a);
		}
		Notify("Capture");
	}else
		a = 0;
	if (Capturing)
		HuiRunLaterM(UPDATE_TIME, this, (void(HuiEventHandler::*)())&AudioInput::Update);
	msg_db_l(1);
	return a;
}

void AudioInput::Update()
{
	CaptureCurrentSamples = DoCapturing();
}

BufferBox AudioInput::GetSomeSamples(int num_samples)
{
	BufferBox buf;
	if (CaptureAddData){
		int n = min(CaptureBuf.num, num_samples);
		int i0 = CaptureBuf.num - n;
		buf.resize(n);
		for (int i=0;i<n;i++){
			buf.r[i] = CaptureBuf.r[i0 + i];
			buf.l[i] = CaptureBuf.l[i0 + i];
		}
	}else{
		//buf.set(CapturePreviewBuf, 0, 1.0f);
		buf = CapturePreviewBuf;
	}
	return buf;
}

float AudioInput::GetSampleRate()
{
	return CaptureSampleRate;
}
