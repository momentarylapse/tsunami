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


#include <portaudio.h>

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



int portAudioInputCallback(const void *input, void *output, unsigned long frameCount, const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags, void *userData)
{
	AudioInputAudio *ia = (AudioInputAudio*)userData;
	if (!ia->isCapturing())
		return paContinue;
	float *in = (float*)input;

	RingBuffer &buf = ia->current_buffer;
	BufferBox b;
	buf.writeRef(b, frameCount);
	b.deinterleave(in);

	unsigned int done = b.num;
	if (done < frameCount){
		buf.writeRef(b, frameCount - done);
		b.deinterleave(&in[2 * done]);
	}

	return paContinue;
}

AudioInputAudio::AudioInputAudio(BufferBox &buf, RingBuffer &cur_buf) :
	accumulation_buffer(buf), current_buffer(cur_buf)
{
	capturing = false;
	pa_device_no = 0;
	accumulating = false;
	memset(capture_temp, 0, sizeof(capture_temp));
	sample_rate = DEFAULT_SAMPLE_RATE;

	chosen_device = HuiConfig.getStr("Input.ChosenDevice", "");
	playback_delay_const = HuiConfig.getFloat("Input.PlaybackDelay", 80.0f);
	temp_filename = HuiConfig.getStr("Input.TempFilename", "");
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
	devices.clear();

	int n = Pa_GetDeviceCount();
	for (int i=0; i<n; i++){
		const PaDeviceInfo *di = Pa_GetDeviceInfo(i);
		if (di->maxInputChannels >= 1)
			devices.add(di->name);
	}

	setDevice(chosen_device);
}

void AudioInputAudio::setDevice(const string &device)
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
		pa_device_no = Pa_GetDefaultInputDevice();
		if (device != "")
			tsunami->log->error(format("input device '%s' not found. Using default.", device.c_str()));
	}

	tsunami->log->info(format("input device '%s' chosen", Pa_GetDeviceInfo(pa_device_no)->name));
}

void AudioInputAudio::stop()
{
	msg_db_f("CaptureStop", 0);
	if (!capturing)
		return;
	last_error = Pa_StopStream(pa_stream);
	testError("Pa_StopStream");
	last_error = Pa_CloseStream(pa_stream);
	testError("Pa_CloseStream");
	capturing = false;
	current_buffer.clear();
	delete(temp_file);
	temp_file = NULL;
	file_delete(cur_temp_filename);
}

bool AudioInputAudio::start(int _sample_rate)
{
	msg_db_f("CaptureStart", 0);
	if (capturing)
		stop();
	msg_write("aaa");

	init();
	accumulating = false;
	sample_rate = _sample_rate;

	PaStreamParameters param;
	bzero(&param, sizeof(param));
	param.channelCount = 2;
	param.device = pa_device_no;
	param.hostApiSpecificStreamInfo = NULL;
	param.sampleFormat = paFloat32;
	param.suggestedLatency = Pa_GetDeviceInfo(pa_device_no)->defaultLowInputLatency;
	param.hostApiSpecificStreamInfo = NULL;
	last_error = Pa_OpenStream(
	                &pa_stream,
	                &param,
	                NULL,
	                sample_rate,
					paFramesPerBufferUnspecified,
	                paNoFlag, //flags that can be used to define dither, clip settings and more
	                portAudioInputCallback,
	                (void*)this);
	testError("Pa_OpenStream");

	last_error = Pa_StartStream(pa_stream);
	testError("Pa_StartStream");
	capturing = true;

	cur_temp_filename = getTempFilename();
	temp_file = FileCreate(getTempFilename());
	temp_file->SetBinaryMode(true);

	resetSync();
	return capturing;
}

bool AudioInputAudio::testError(const string &msg)
{
	if (last_error != paNoError){
		tsunami->log->error(format(_("PortAudio (input) error: '%s'  at %s"), Pa_GetErrorText(last_error), msg.c_str()));
		return true;
	}
	return false;
}

float AudioInputAudio::getPlaybackDelayConst()
{
	return playback_delay_const;
}

void AudioInputAudio::accumulate(bool enable)
{
	current_buffer.clear();
	accumulating = enable;
}

void AudioInputAudio::resetAccumulation()
{
	accumulation_buffer.clear();
}

int AudioInputAudio::getSampleCount()
{
	return accumulation_buffer.num;
}

void AudioInputAudio::setPlaybackDelayConst(float f)
{
	playback_delay_const = f;
	HuiConfig.setFloat("Input.PlaybackDelay", playback_delay_const);
}

int AudioInputAudio::doCapturing()
{
	msg_db_f("DoCapturing", 1);

	int avail = current_buffer.available();
	sync.add(avail);

	if (!accumulating)
		return avail;

	BufferBox b;
	current_buffer.readRef(b, avail);
	accumulation_buffer.append(b);

	// write to file
	Array<short> data;
	b.get_16bit_buffer(data);
	temp_file->WriteBuffer(&data[0], b.num * 4);

	return avail;
}

void AudioInputAudio::resetSync()
{
	sync.reset();
}

void AudioInputAudio::getSomeSamples(BufferBox &buf, int num_samples)
{
	current_buffer.peekRef(buf, num_samples);
}

float AudioInputAudio::getSampleRate()
{
	return sample_rate;
}

bool AudioInputAudio::isCapturing()
{
	return capturing;
}

int AudioInputAudio::getDelay()
{
	return sync.getDelay() - playback_delay_const * (float)sample_rate / 1000.0f;
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
	if (temp_filename.num > 0)
		return temp_filename;
	return getDefaultTempFilename();
}

void AudioInputAudio::setTempFilename(const string &filename)
{
	temp_filename = filename;
	HuiConfig.setStr("Input.TempFilename", temp_filename);
}
