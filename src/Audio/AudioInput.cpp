/*
 * AudioInput.cpp
 *
 *  Created on: 26.03.2012
 *      Author: michi
 */

#include "AudioInput.h"
#include "AudioOutput.h"
#include "../lib/hui/hui.h"
#include "../Tsunami.h"
#include "../Stuff/Log.h"


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

void AudioInput::SyncData::Reset()
{
	num_points = 0;
	delay_sum = 0;
	samples_in = 0;
	if (tsunami->output->IsPlaying())
		offset_out = tsunami->output->GetRange().offset;
}

void AudioInput::SyncData::Add(int samples)
{
	if (tsunami->output->IsPlaying()){
		samples_in += samples;
		delay_sum += (tsunami->output->GetPos() - offset_out - samples_in);
		num_points ++;
	}
}

int AudioInput::SyncData::GetDelay()
{
	if (num_points > 0)
		return (delay_sum / num_points);
	return 0;
}

AudioInput::AudioInput() :
	PeakMeterSource("AudioInput")
{
	Capturing = false;
	capture = NULL;
	memset(capture_temp, 0, sizeof(capture_temp));
	SampleRate = DEFAULT_SAMPLE_RATE;

	ChosenDevice = HuiConfigReadStr("Input.ChosenDevice", "");
	PlaybackDelayConst = HuiConfigReadFloat("Input.PlaybackDelay", 80.0f);

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
		alcCaptureStop(capture);
		alcCaptureCloseDevice(capture);
		Capturing = false;
		CurrentBuffer.clear();
	}
	msg_db_l(1);
}

bool AudioInput::Start(int sample_rate)
{
	msg_db_r("CaptureStart", 1);
	if (Capturing)
		Stop();

	Init();
	SampleRate = sample_rate;
	capture = alcCaptureOpenDevice(dev_name.c_str(), sample_rate, AL_FORMAT_STEREO16, NUM_CAPTURE_SAMPLES);
	//msg_write((int)capture);
	if (capture){
		alcCaptureStart(capture);
		Capturing = true;
		HuiRunLaterM(UPDATE_TIME, this, &AudioInput::Update);
	}
	if (!Capturing)
		tsunami->log->Error(_("Konnte Aufnahmeger&at nicht &offnen"));
	ResetSync();
	msg_db_l(1);
	return Capturing;
}

float AudioInput::GetPlaybackDelayConst()
{
	return PlaybackDelayConst;
}

void AudioInput::SetPlaybackDelayConst(float f)
{
	PlaybackDelayConst = f;
	HuiConfigWriteFloat("Input.PlaybackDelay", PlaybackDelayConst);
}

int AudioInput::DoCapturing()
{
	msg_db_r("DoCapturing", 1);
	int a = -42;
	alcGetIntegerv(capture, ALC_CAPTURE_SAMPLES, 1, &a);

	// don't wait, till we really have as much data as we requested
	//   (or else it might freeze up....)
	if (a >= NUM_CAPTURE_SAMPLES / 8){
		bool too_much_data = (a > NUM_CAPTURE_SAMPLES);
		if (too_much_data)
			a = NUM_CAPTURE_SAMPLES;

		alcCaptureSamples(capture, capture_temp, a);

		CurrentBuffer.resize(a);
		CurrentBuffer.set_16bit(capture_temp, 0, a);

		if (!too_much_data)
			sync.Add(a);

		Notify("Capture");
	}else
		a = 0;
	msg_db_l(1);
	return a;
}

void AudioInput::Update()
{
	DoCapturing();
	if (Capturing)
		HuiRunLaterM(UPDATE_TIME, this, &AudioInput::Update);
}

void AudioInput::ResetSync()
{
	sync.Reset();
}

BufferBox AudioInput::GetSomeSamples(int num_samples)
{
	BufferBox buf;
	num_samples = min(num_samples, CurrentBuffer.num);
	buf.set_as_ref(CurrentBuffer, CurrentBuffer.num - num_samples, num_samples);
	return buf;
}

float AudioInput::GetSampleRate()
{
	return SampleRate;
}

bool AudioInput::IsCapturing()
{
	return Capturing;
}

int AudioInput::GetDelay()
{
	return sync.GetDelay() - PlaybackDelayConst * (float)SampleRate / 1000.0f;
}
