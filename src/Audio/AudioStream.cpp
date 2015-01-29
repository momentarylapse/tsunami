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
		int done = 0;

		for (int n=0; (n<2) and (done<frameCount); n++){
			BufferBox b;
			stream->ring_buf.readRef(b, frameCount - done);
			b.interleave(out);
			out += done * 2;
			done += b.num;
		}

		stream->cur_pos += done;
	}

	// read more?
	if ((available < stream->buffer_size) and (!stream->reading) and (!stream->end_of_data)){
		stream->reading = true;
		HuiRunLaterM(0, stream, &AudioStream::stream);
	}

	if (available <= frameCount and stream->end_of_data){
		HuiRunLaterM(0, stream, &AudioStream::stop); // TODO prevent abort before playback really finished
		return paComplete;
	}
	return paContinue;
}

AudioStream::AudioStream(AudioRendererInterface *r) :
	PeakMeterSource("AudioStream"),
	ring_buf(1048576)
{
	last_error = paNoError;

	renderer = r;

	playing = false;
	paused = false;
	volume = 1;
	reading = false;

	output = tsunami->output;

	data_samples = 0;
	buffer_size = DEFAULT_BUFFER_SIZE;
	sample_rate = DEFAULT_SAMPLE_RATE;
	update_dt = DEFAULT_UPDATE_DT;

	int outDevNum = output->pa_device_no;

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

void AudioStream::__init__(AudioRendererInterface *r)
{
	new(this) AudioStream(r);
}

void AudioStream::__delete__()
{
	kill();
}

void AudioStream::kill()
{
	msg_db_f("Stream.kill", 1);
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
	//last_error = Pa_StopStream(pa_stream);
	testError("AudioStream.stop");

	// clean up
	playing = false;
	paused = false;
	end_of_data = false;
	ring_buf.clear();

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

	// read data
	size = renderer->read(b);

	// out of data?
	if (size == 0){
		end_of_data = true;
		reading = false;
		return;
	}

	// add to queue
	ring_buf.write(b);

	reading = false;
}

void AudioStream::setSource(AudioRendererInterface *r)
{
	msg_db_f("Stream.setSource", 1);

	if (playing)
		stop();

	renderer = r;
	sample_rate = r->sample_rate;
}

void AudioStream::play()
{
	msg_db_f("Stream.play", 1);

	if (playing){
		/*if (paused){
			pause();
			return;
		}*/
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

	pos = cur_pos;

	// translation
	Range r = renderer->range();
	if (r.num > 0)
		pos = r.offset + ((cur_pos + renderer->offset()) %r.num);
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

	ring_buf.peekRef(buf, num_samples);
}

bool AudioStream::testError(const string &msg)
{
	return output->testError(msg);
}

void AudioStream::update()
{
	// <this> got deleted, while a timeout was still pending?
	if (!tsunami->output->streamExists(this))
		return;

	msg_db_f("Stream.update", 1);
	testError("idle");
	if (!playing)
		return;

	if (!paused)
		notify(MESSAGE_UPDATE);

	HuiRunLaterM(update_dt, this, &AudioStream::update);
}


