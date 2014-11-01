/*
 * AudioInputAudio.cpp
 *
 *  Created on: 26.03.2012
 *      Author: michi
 */

#include "AudioInputAudio.h"
#include "AudioStream.h"
#include "../lib/hui/hui.h"
#include "../Tsunami.h"
#include "../TsunamiWindow.h"
#include "../Stuff/Log.h"
#include "../View/AudioView.h"


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

void AudioInputAudio::SyncData::reset()
{
	num_points = 0;
	delay_sum = 0;
	samples_in = 0;
	/*if (tsunami->output->IsPlaying())
		offset_out = tsunami->output->GetRange().offset;*/ // TODO
}

void AudioInputAudio::SyncData::add(int samples)
{
	if (tsunami->win->view->stream->isPlaying()){
		samples_in += samples;
		/*delay_sum += (tsunami->output->GetPos() - offset_out - samples_in);*/ // TODO
		num_points ++;
	}
}

int AudioInputAudio::SyncData::getDelay()
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
	accumulating = false;
	memset(capture_temp, 0, sizeof(capture_temp));
	SampleRate = DEFAULT_SAMPLE_RATE;

	ChosenDevice = HuiConfig.getStr("Input.ChosenDevice", "");
	PlaybackDelayConst = HuiConfig.getFloat("Input.PlaybackDelay", 80.0f);
	TempFilename = HuiConfig.getStr("Input.TempFilename", "");
	temp_file = NULL;

	if (file_test_existence(getTempFilename()))
		tsunami->log->warning(_("alte Aufnahmedaten gefunden: ") + getTempFilename());
}

AudioInputAudio::~AudioInputAudio()
{
}


void AudioInputAudio::init()
{
	msg_db_f("CaptureInit", 1);
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
}

void AudioInputAudio::stop()
{
	msg_db_f("CaptureStop", 1);
	if (Capturing){
		alcCaptureStop(capture);
		alcCaptureCloseDevice(capture);
		Capturing = false;
		CurrentBuffer.clear();
		delete(temp_file);
		temp_file = NULL;
		file_delete(cur_temp_filename);
	}
}

bool AudioInputAudio::start(int sample_rate)
{
	msg_db_f("CaptureStart", 1);
	if (Capturing)
		stop();

	init();
	accumulating = false;
	SampleRate = sample_rate;
	capture = alcCaptureOpenDevice(dev_name.c_str(), sample_rate, AL_FORMAT_STEREO16, NUM_CAPTURE_SAMPLES);
	//msg_write((int)capture);
	if (capture){
		alcCaptureStart(capture);
		Capturing = true;

		cur_temp_filename = getTempFilename();
		temp_file = FileCreate(getTempFilename());
		temp_file->SetBinaryMode(true);
	}
	resetSync();
	return Capturing;
}

float AudioInputAudio::getPlaybackDelayConst()
{
	return PlaybackDelayConst;
}

void AudioInputAudio::accumulate(bool enable)
{
	accumulating = enable;
}

void AudioInputAudio::resetAccumulation()
{
	AccumulationBuffer.clear();
}

int AudioInputAudio::getSampleCount()
{
	return AccumulationBuffer.num;
}

void AudioInputAudio::setPlaybackDelayConst(float f)
{
	PlaybackDelayConst = f;
	HuiConfig.setFloat("Input.PlaybackDelay", PlaybackDelayConst);
}

int AudioInputAudio::doCapturing()
{
	msg_db_f("DoCapturing", 1);
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
			sync.add(a);

		if (accumulating){
			AccumulationBuffer.append(CurrentBuffer);
			temp_file->WriteBuffer(&capture_temp, a * 4);
		}
	}else
		a = 0;
	return a;
}

void AudioInputAudio::resetSync()
{
	sync.reset();
}

void AudioInputAudio::getSomeSamples(BufferBox &buf, int num_samples)
{
	num_samples = min(num_samples, CurrentBuffer.num);
	buf.set_as_ref(CurrentBuffer, CurrentBuffer.num - num_samples, num_samples);
}

float AudioInputAudio::getSampleRate()
{
	return SampleRate;
}

bool AudioInputAudio::isCapturing()
{
	return Capturing;
}

int AudioInputAudio::getDelay()
{
	return sync.getDelay() - PlaybackDelayConst * (float)SampleRate / 1000.0f;
}

string AudioInputAudio::getDefaultTempFilename()
{
#ifdef OS_WINDOWS
	return "c:\\tsunami-input.raw";
#else
	return "/tmp/tsunami-input.raw";
#endif
}

string AudioInputAudio::getTempFilename()
{
	if (TempFilename.num > 0)
		return TempFilename;
	return getDefaultTempFilename();
}

void AudioInputAudio::setTempFilename(const string &filename)
{
	TempFilename = filename;
	HuiConfig.setStr("Input.TempFilename", TempFilename);
}
