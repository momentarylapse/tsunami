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
#include <portaudio.h>
#include <math.h>
#include "../lib/threads/Thread.h"

//#define DEFAULT_BUFFER_SIZE		131072
#define DEFAULT_BUFFER_SIZE		32768
//#define DEFAULT_BUFFER_SIZE		16384

#define DEFAULT_UPDATE_DT		0.050f


const string AudioStream::MESSAGE_STATE_CHANGE = "StateChange";
const string AudioStream::MESSAGE_UPDATE = "Update";

bool AudioStream::JUST_FAKING_IT = false;

int portAudioCallback(const void *input, void *output, unsigned long frameCount, const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags, void *userData)
{
	AudioStream *stream = (AudioStream*)userData;
	float *out = (float*)output;
//	printf("%d\n", frameCount);

	if (statusFlags != 0)
		printf("flags: %d\n", statusFlags);
	//paOutputUnderflow

	unsigned int available = stream->ring_buf.available();
	if ((stream->paused) or (available < frameCount)){
		printf("<\n");
		// output silence...
		for (unsigned int i=0; i<frameCount; i++){
			*out ++ = 0;
			*out ++ = 0;
		}
	}else{
		unsigned int done = 0;

		for (int n=0; (n<2) and (done<frameCount); n++){
			BufferBox b;
			stream->ring_buf.readRef(b, frameCount - done);
			b.interleave(out, stream->output->getVolume() * stream->volume);
			out += b.num * 2;
			done += b.num;
			break;
		}

		stream->cur_pos += done;
	}

	// read more?
	if ((available < (unsigned)stream->buffer_size) and (!stream->reading) and (!stream->read_more) and (!stream->end_of_data)){
//		printf("+\n");
		stream->read_more = true;
	}

	if (available <= frameCount and stream->end_of_data){
//		printf("end\n");
		HuiRunLaterM(0.001f, stream, &AudioStream::stop); // TODO prevent abort before playback really finished
		return paComplete;
	}
	return paContinue;
}

class StreamThread : public Thread
{
public:
	AudioStream *stream;
	HuiTimer timer;
	float t_idle;

	StreamThread(AudioStream *s)
	{
		stream = s;
		t_idle = 0;
	}

	virtual void onRun()
	{
		timer.reset();
		while(true){
			if (stream->read_more){
				stream->stream();
				float t_busy = timer.get();
				stream->cpu_usage = t_busy / (t_busy + t_idle);
				//printf("%.1f %%\n", stream->cpu_usage * 100);
				t_idle = 0;
			}else{
				HuiSleep(0.005f);
				t_idle += timer.get();
			}
		}
	}
};

AudioStream::AudioStream(AudioRendererInterface *r) :
	PeakMeterSource("AudioStream"),
	ring_buf(1048576)
{
	last_error = paNoError;

	renderer = r;

	playing = false;
	paused = false;
	volume = 1;
	read_more = false;
	reading = false;

	output = tsunami->output;

	data_samples = 0;
	buffer_size = DEFAULT_BUFFER_SIZE;
	update_dt = DEFAULT_UPDATE_DT;
	killed = false;
	thread = NULL;
	pa_stream = NULL;
	dev_sample_rate = -1;
	cpu_usage = 0;
	end_of_data = false;
	cur_pos = 0;

	if (JUST_FAKING_IT)
		return;

	output->addStream(this);
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

void AudioStream::create_dev()
{
	if (pa_stream)
		return;

	int outDevNum = output->pa_device_no;

	PaStreamParameters outputParameters;
	bzero(&outputParameters, sizeof(outputParameters));
	outputParameters.channelCount = 2;
	outputParameters.device = outDevNum;
	outputParameters.hostApiSpecificStreamInfo = NULL;
	outputParameters.sampleFormat = paFloat32;
	outputParameters.suggestedLatency = Pa_GetDeviceInfo(outDevNum)->defaultLowOutputLatency;
	outputParameters.hostApiSpecificStreamInfo = NULL;
	dev_sample_rate = renderer->getSampleRate();
	last_error = Pa_OpenStream(
	                &pa_stream,
	                NULL,
	                &outputParameters,
					dev_sample_rate,
					512,//paFramesPerBufferUnspecified,
	                paNoFlag, //flags that can be used to define dither, clip settings and more
	                portAudioCallback,
	                (void*)this);
	testError("Pa_OpenStream");
}

void AudioStream::kill_dev()
{
	msg_db_f("Stream.kill_dev", 1);

	if (pa_stream){
		stop();

		last_error = Pa_CloseStream(pa_stream);
		testError("AudioStream.kill");
		pa_stream = NULL;
	}
}

void AudioStream::kill()
{
	msg_db_f("Stream.kill", 1);

	if (pa_stream)
		kill_dev();

	output->removeStream(this);
	killed = true;

	if (thread){
		thread->kill();
		delete(thread);
		thread = NULL;
	}
}

void AudioStream::stop()
{
	if (!playing)
		return;
	msg_db_f("Stream.stop", 1);

	//last_error = Pa_AbortStream(pa_stream);
	last_error = Pa_StopStream(pa_stream);
	testError("Pa_StopStream");

	// clean up
	playing = false;
	paused = false;
	end_of_data = false;
	read_more = false;
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
	read_more = false;
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
}

void AudioStream::play()
{
	msg_db_f("Stream.play", 1);

	if (dev_sample_rate != renderer->getSampleRate())
		kill_dev();
	if (!pa_stream)
		create_dev();


	if (playing){
		/*if (paused){
			pause();
			return;
		}*/
		stop();
	}

	if (!thread){
		thread = new StreamThread(this);
		thread->run();
	}


	end_of_data = false;
	reading = false;

	renderer->reset();
	stream();
	last_error = Pa_StartStream(pa_stream);
	testError("Pa_StartStream");

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
	return renderer->getSampleRate();
}

void AudioStream::getSomeSamples(BufferBox &buf, int num_samples)
{
	if (!playing)
		return;

	ring_buf.peekRef(buf, num_samples);
}

bool AudioStream::testError(const string &msg)
{
	if (last_error != paNoError){
		tsunami->log->error(format(_("PortAudio (stream) error: '%s'   at %s"), Pa_GetErrorText(last_error), msg.c_str()));
		return true;
	}
	return false;
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


