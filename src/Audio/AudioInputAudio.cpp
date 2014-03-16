/*
 * AudioInputAudio.cpp
 *
 *  Created on: 26.03.2012
 *      Author: michi
 */

#include "AudioInputAudio.h"
#include "AudioOutput.h"
#include "../lib/hui/hui.h"
#include "../Tsunami.h"


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

void AudioInputAudio::SyncData::Reset()
{
	num_points = 0;
	delay_sum = 0;
	samples_in = 0;
	/*if (tsunami->output->IsPlaying())
		offset_out = tsunami->output->GetRange().offset;*/ // TODO
}

void AudioInputAudio::SyncData::Add(int samples)
{
	if (tsunami->output->IsPlaying()){
		samples_in += samples;
		/*delay_sum += (tsunami->output->GetPos() - offset_out - samples_in);*/ // TODO
		num_points ++;
	}
}

int AudioInputAudio::SyncData::GetDelay()
{
	if (num_points > 0)
		return (delay_sum / num_points);
	return 0;
}

AudioInputAudio::AudioInputAudio(BufferBox &buf, BufferBox &cur_buf) :
	AccumulationBuffer(buf), CurrentBuffer(cur_buf)
{
	Capturing = false;
	capture = NULL;
	memset(capture_temp, 0, sizeof(capture_temp));
	SampleRate = DEFAULT_SAMPLE_RATE;

	ChosenDevice = HuiConfig.getStr("Input.ChosenDevice", "");
	PlaybackDelayConst = HuiConfig.getFloat("Input.PlaybackDelay", 80.0f);

}

AudioInputAudio::~AudioInputAudio()
{
}


void AudioInputAudio::Init()
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

void AudioInputAudio::Stop()
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

bool AudioInputAudio::Start(int sample_rate)
{
	msg_db_r("CaptureStart", 1);
	if (Capturing)
		Stop();

	Init();
	accumulate = false;
	SampleRate = sample_rate;
	capture = alcCaptureOpenDevice(dev_name.c_str(), sample_rate, AL_FORMAT_STEREO16, NUM_CAPTURE_SAMPLES);
	//msg_write((int)capture);
	if (capture){
		alcCaptureStart(capture);
		Capturing = true;
	}
	ResetSync();
	msg_db_l(1);
	return Capturing;
}

float AudioInputAudio::GetPlaybackDelayConst()
{
	return PlaybackDelayConst;
}

void AudioInputAudio::Accumulate(bool enable)
{
	accumulate = enable;
}

void AudioInputAudio::ResetAccumulation()
{
	AccumulationBuffer.clear();
}

int AudioInputAudio::GetSampleCount()
{
	return AccumulationBuffer.num;
}

void AudioInputAudio::SetPlaybackDelayConst(float f)
{
	PlaybackDelayConst = f;
	HuiConfig.setFloat("Input.PlaybackDelay", PlaybackDelayConst);
}

int AudioInputAudio::DoCapturing()
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

		if (accumulate)
			AccumulationBuffer.append(CurrentBuffer);
	}else
		a = 0;
	msg_db_l(1);
	return a;
}

void AudioInputAudio::ResetSync()
{
	sync.Reset();
}

void AudioInputAudio::GetSomeSamples(BufferBox &buf, int num_samples)
{
	num_samples = min(num_samples, CurrentBuffer.num);
	buf.set_as_ref(CurrentBuffer, CurrentBuffer.num - num_samples, num_samples);
}

float AudioInputAudio::GetSampleRate()
{
	return SampleRate;
}

bool AudioInputAudio::IsCapturing()
{
	return Capturing;
}

int AudioInputAudio::GetDelay()
{
	return sync.GetDelay() - PlaybackDelayConst * (float)SampleRate / 1000.0f;
}
