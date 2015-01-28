/*
 * AudioStream.cpp
 *
 *  Created on: 01.11.2014
 *      Author: michi
 */

#include "AudioStream.h"
#include "AudioOutput.h"
#include "../Tsunami.h"
#include "AudioRenderer.h"
#include "../Stuff/Log.h"


#ifdef OS_WINDOWS
	#include <al.h>
	#include <alut.h>
	#include <alc.h>
	#pragma comment(lib,"alut.lib")
	#pragma comment(lib,"OpenAL32.lib")

#else
	#include <portaudio.h>
#endif
#include <math.h>

//#define DEFAULT_BUFFER_SIZE		131072
#define DEFAULT_BUFFER_SIZE		32768
//#define DEFAULT_BUFFER_SIZE		16384

#define DEFAULT_UPDATE_DT		0.050f


const string AudioStream::MESSAGE_STATE_CHANGE = "StateChange";
const string AudioStream::MESSAGE_UPDATE = "Update";

int portAudioCallback(const void *input, void *output, unsigned long frameCount, const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags, void *userData)
{
	AudioStream *stream = (AudioStream*)userData;
	float *out = (float*)output;

	int available = stream->ring_buf.available();
	if ((stream->paused) or (available < frameCount)){
		// output silence...
		for (unsigned int i=0; i<frameCount; i++){
			*out ++ = 0;
			*out ++ = 0;
		}
	}else{
		stream->cur_pos += frameCount;

		int na = min(stream->ring_buf.buf.num - stream->ring_buf.read_pos, frameCount);
		float *r = &stream->ring_buf.buf.r[stream->ring_buf.read_pos];
		float *l = &stream->ring_buf.buf.l[stream->ring_buf.read_pos];
		for (int i=0; i<na; i++){
			*out ++ = *r ++;
			*out ++ = *l ++;
		}
		stream->ring_buf.read_pos += frameCount;
		if (stream->ring_buf.read_pos > stream->ring_buf.buf.num)
			stream->ring_buf.read_pos -= stream->ring_buf.buf.num;
	}

	// read more?
	if ((available < stream->buffer_size) and (!stream->reading)){
		stream->reading = true;
		HuiRunLaterM(0, stream, &AudioStream::stream);
	}
	return 0;
}

AudioStream::AudioStream() :
	PeakMeterSource("AudioStream"),
	ring_buf(1048576)
{
	last_error = paNoError;

	renderer = NULL;
	generate_func = NULL;

	playing = false;
	paused = false;
	volume = 1;
	reading = false;

	output = tsunami->output;

	data_samples = 0;
	buffer_size = DEFAULT_BUFFER_SIZE;
	sample_rate = DEFAULT_SAMPLE_RATE;
	update_dt = DEFAULT_UPDATE_DT;

	int outDevNum = Pa_GetDefaultOutputDevice();

	PaStreamParameters outputParameters;
	bzero(&outputParameters, sizeof(outputParameters));
	outputParameters.channelCount = 2;
	outputParameters.device = outDevNum;
	outputParameters.hostApiSpecificStreamInfo = NULL;
	outputParameters.sampleFormat = paFloat32;
	outputParameters.suggestedLatency = Pa_GetDeviceInfo(outDevNum)->defaultLowOutputLatency;
	outputParameters.hostApiSpecificStreamInfo = NULL;
	last_error = Pa_OpenStream(
	                &pa_stream,
	                NULL,
	                &outputParameters,
	                sample_rate,
					paFramesPerBufferUnspecified,
	                paNoFlag, //flags that can be used to define dither, clip settings and more
	                portAudioCallback,
	                (void*)this);
	testError("OpenStream");

	output->addStream(this);
	killed = false;
}

AudioStream::~AudioStream()
{
	kill();
}

void AudioStream::__init__()
{
	new(this) AudioStream;
}

void AudioStream::__delete__()
{
	this->~AudioStream();
}

void AudioStream::kill()
{
	if (killed)
		return;

	stop();

	last_error = Pa_CloseStream(&pa_stream);
	testError("AudioStream.kill");

	output->removeStream(this);
	killed = true;
}

void AudioStream::stop()
{
	if (!playing)
		return;
	msg_db_f("Stream.stop", 1);

	last_error = Pa_AbortStream(pa_stream);
	testError("AudioStream.stop");

	playing = false;
	paused = false;

	notify(MESSAGE_STATE_CHANGE);
}

void AudioStream::pause()
{
	if (!playing)
		return;

	paused = !paused;
	notify(MESSAGE_STATE_CHANGE);
}

void AudioStream::stream()
{
	reading = true;
	msg_db_f("stream", 1);

	int size = 0;
	BufferBox b;
	b.resize(buffer_size);
	msg_write("stream");

	// read data
	if (renderer){
		size = renderer->read(b);
	}else if (generate_func){
		size = (*generate_func)(b);
	}
	msg_write(size);

	// out of data?
	if (size == 0){
		msg_write("end of data");
		end_of_data = true;
		reading = false;
		return;
	}

	// add to queue
	ring_buf.write(b);
	msg_write(ring_buf.available());

	reading = false;
}

void AudioStream::setSource(AudioRendererInterface *r)
{
	msg_db_f("Stream.setSource", 1);

	if (playing)
		stop();

	renderer = r;
	generate_func = NULL;
	sample_rate = r->sample_rate;
}

void AudioStream::setSourceGenerated(void *func, int _sample_rate)
{
	msg_db_f("Stream.setSourceGen", 1);

	if (playing)
		stop();

	renderer = NULL;
	generate_func = (generate_func_t*)func;
	sample_rate = _sample_rate;
}

void AudioStream::play()
{
	msg_db_f("Stream.play", 1);

	if (playing){
		if (paused){
			pause();
			return;
		}
		stop();
	}


	end_of_data = false;
	reading = false;

	stream();
	last_error = Pa_StartStream(pa_stream);
	testError("StartStream");

	playing = true;
	paused = false;
	cur_pos = 0;

	HuiRunLaterM(update_dt, this, &AudioStream::update);
	notify(MESSAGE_STATE_CHANGE);
}

bool AudioStream::isPlaying()
{
	return playing;
}

int AudioStream::getState()
{
	if (playing and paused)
		return STATE_PAUSED;
	if (playing)
		return STATE_PLAYING;
	/*if (Pa_IsStreamActive(pa_stream))
		return STATE_PLAYING;*/
	/*if (IsPlaying())
		return STATE_PLAYING;*/
	/*int param;
	alGetSourcei(source, AL_SOURCE_STATE, &param);
	if (param == AL_PLAYING)
		return STATE_PLAYING;
	if (param == AL_PAUSED)
		return STATE_PAUSED;*/
	return STATE_STOPPED;
}

int AudioStream::getPos()
{
	int pos;
	if (getPosSafe(pos))
		return pos;
	return 0;
}

bool AudioStream::getPosSafe(int &pos)
{
	if (!playing)
		return false;

	//pos = (Pa_GetStreamTime(pa_stream) - pa_time_offset) * sample_rate;
	//msg_write(f2s(Pa_GetStreamTime(pa_stream), 4));
	pos = cur_pos;
/*	int param = 0;
	alGetSourcei(source, AL_SOURCE_STATE, &param);
	testError("alGetSourcei1 (getpos)");
	if ((param != AL_PLAYING) and (param != AL_PAUSED))
		return false;

	alGetSourcei(source, AL_SAMPLE_OFFSET, &param);
	testError("alGetSourcei2 (getpos)");
	pos = box[cur_buffer_no].offset + param;*/
	return true;
}

float AudioStream::getVolume()
{
	return volume;
}

void AudioStream::setVolume(float _volume)
{
	volume = _volume;
	notify(MESSAGE_STATE_CHANGE);
}

float AudioStream::getSampleRate()
{
	return sample_rate;
}

void AudioStream::getSomeSamples(BufferBox &buf, int num_samples)
{
	if (!playing)
		return;

	// (sample) position within current stream/buffer
	/*int dpos = 0;
	alGetSourcei(source, AL_SAMPLE_OFFSET, &dpos);

	if (box[cur_buffer_no].num - dpos > 0)
		buf.set_as_ref(box[cur_buffer_no], dpos, min(num_samples, box[cur_buffer_no].num - dpos));
	else
		buf.set_as_ref(box[1-cur_buffer_no], 0, min(num_samples, box[1-cur_buffer_no].num));*/
}

bool AudioStream::testError(const string &msg)
{
	return output->testError(msg);
}

void AudioStream::update()
{
	msg_db_f("Stream.update", 1);
	testError("idle");
	if (!playing)
		return;

	if (!paused)
		notify(MESSAGE_UPDATE);

	HuiRunLaterM(update_dt, this, &AudioStream::update);
}


