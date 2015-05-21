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
#include <pulse/pulseaudio.h>
#include <math.h>
#include "../lib/threads/Thread.h"

//#define DEFAULT_BUFFER_SIZE		131072
#define DEFAULT_BUFFER_SIZE		32768
//#define DEFAULT_BUFFER_SIZE		16384

#define DEFAULT_UPDATE_DT		0.050f


const string AudioStream::MESSAGE_STATE_CHANGE = "StateChange";
const string AudioStream::MESSAGE_UPDATE = "Update";

bool AudioStream::JUST_FAKING_IT = false;

#if 0
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
#endif


void pa_wait_op(pa_operation *op)
{
	msg_write(p2s(op));
	while (pa_operation_get_state(op) != PA_OPERATION_DONE)
		{}//pa_mainloop_iterate(m, 1, NULL);
	pa_operation_unref(op);
	msg_write(" ok");
}

static float _offset__ = 0;

static void stream_request_callback(pa_stream *p, size_t nbytes, void *userdata)
{
	int nbytes0 = nbytes;
	printf("request %d\n", (int)nbytes);
	AudioStream *stream = (AudioStream*)userdata;

	void *data;
	int r = pa_stream_begin_write(p, &data, &nbytes);
	stream->testError("begin write");
	printf("%d  %p  %d\n", r, data, (int)nbytes);
	//if (nbytes > nbytes0)
	//	nbytes = nbytes0;

	if (!stream->playing){
		msg_error("not playing");
		pa_stream_cancel_write(p);
		stream->testError("cancel write");
		return;
	}

	int done = 0;
	int frames = nbytes / 8;
	float *out = (float*)data;

	for (int i=0; i<frames; i++){
		out[i*2  ] = sin(_offset__) * 0.02f;
		out[i*2+1] = sin(_offset__) * 0.02f;
		_offset__ += 0.04f;
		if (_offset__ > 2*3.141592f)
			_offset__ -= 2*3.141592f;
	}

	/*while (frames > stream->ring_buf.available()){
		if (stream->end_of_data){
			HuiRunLaterM(0.001f, stream, &AudioStream::stop); // TODO prevent abort before playback really finished
			break;
		}
		msg_write("----------need more");
		stream->stream();
	}

	for (int n=0; (n<2) and (done < frames); n++){
		BufferBox b;
		stream->ring_buf.readRef(b, frames - done);
		b.interleave(out, stream->output->getVolume() * stream->volume);
		out += b.num * 2;
		done += b.num;
		break;
	}*/
	done = frames;

	stream->cur_pos += done;

	pa_stream_write(p, data, nbytes, NULL, 0, (pa_seek_mode_t)PA_SEEK_RELATIVE);
	stream->testError("write");

	/*int r = pa_stream_peek(p, &data, &nbytes);

	if (data){
		//msg_write(pa_stream_writable_size((pa_stream*)userdata));
		r = pa_stream_write((pa_stream*)userdata, data, nbytes, NULL, 0, (pa_seek_mode_t)PA_SEEK_RELATIVE);
		//msg_write(r);
		pa_stream_drop(p);
	}
	//msg_write(">");*/
}

static void _pa_stream_success_cb(pa_stream *s, int success, void *userdata)
{
	msg_write("--success");
}

static void _pa_stream_underflow_cb(pa_stream *s, void *userdata)
{
	msg_error("underflow");

	AudioStream *stream = (AudioStream*)userdata;


	/*stream_request_callback(s, stream->buffer_size, stream);

	msg_write("trigger out");
	pa_operation *op = pa_stream_trigger(s, &_pa_stream_success_cb, NULL);
	stream->testError("trigger");
	pa_wait_op(op);*/
	msg_write(pa_stream_get_state(s));
}

/*class StreamThread : public Thread
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
};*/

AudioStream::AudioStream(AudioRendererInterface *r) :
	PeakMeterSource("AudioStream"),
	ring_buf(1048576)
{
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
	_stream = NULL;
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
	if (_stream)
		return;

	dev_sample_rate = renderer->getSampleRate();

	pa_sample_spec ss;
	ss.rate = dev_sample_rate;
	ss.channels = 2;
	ss.format = PA_SAMPLE_FLOAT32LE;
	//ss.format = PA_SAMPLE_S16LE;
	_stream = pa_stream_new(output->context, "stream", &ss, NULL);
	testError("stream new");

	pa_stream_set_write_callback(_stream, &stream_request_callback, this);
	pa_stream_set_underflow_callback(_stream, &_pa_stream_underflow_cb, this);
	//pa_stream_set_underflow_callback(_stream, &_pa_stream_underflow_cb, this);
}

void AudioStream::kill_dev()
{
	msg_db_f("Stream.kill_dev", 1);

	if (_stream){
		stop();

		pa_stream_unref(_stream);
		testError("unref");
		_stream = NULL;
	}
}

void AudioStream::kill()
{
	msg_db_f("Stream.kill", 1);

	if (_stream)
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

	playing = false;

	//last_error = Pa_AbortStream(pa_stream);
	if (_stream){

		pa_operation *op = pa_stream_drain(_stream, NULL, NULL);
		testError("drain");
		pa_wait_op(op);

		pa_stream_disconnect(_stream);
		testError("disconnect");

		kill_dev();
	}

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

	/*if (dev_sample_rate != renderer->getSampleRate())
		kill_dev();
	if (!_stream)
		create_dev();
	if (!_stream)
		return;*/


	if (playing){
		/*if (paused){
			pause();
			return;
		}*/
		stop();
	}

	/*if (!thread){
		thread = new StreamThread(this);
		thread->run();
	}*/



	end_of_data = false;
	reading = false;


	playing = true;
	paused = false;
	cur_pos = 0;

	renderer->reset();
	stream();


	create_dev();
	if (!_stream)
		return;

	pa_buffer_attr attr_out;
	attr_out.fragsize = 1024;//-1;//512;
	attr_out.maxlength = 4096;
	attr_out.minreq = -1;
	attr_out.tlength = -1;
	attr_out.prebuf = -1;
	pa_stream_connect_playback(_stream, NULL, &attr_out, (pa_stream_flags)0, NULL, NULL);
	testError("connect");



	msg_write("wait out");
	msg_write(pa_stream_get_state(_stream));
	while (pa_stream_get_state(_stream) != PA_STREAM_READY)
		{}//pa_mainloop_iterate(m, 1, NULL);
	msg_write(pa_stream_get_state(_stream));
	msg_write("ok");

	//stream_request_callback(_stream, ring_buf.available(), this);

	msg_write("trigger out");
	pa_operation *op = pa_stream_trigger(_stream, &_pa_stream_success_cb, NULL);
	testError("trigger");
	pa_wait_op(op);

	//HuiRunLaterM(update_dt, this, &AudioStream::update);
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
	int e = pa_context_errno(output->context);
	msg_write(msg + ": " + pa_strerror(e));
	return (e != 0);
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


