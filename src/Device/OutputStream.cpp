/*
 * OutputStream.cpp
 *
 *  Created on: 01.11.2014
 *      Author: michi
 */

#include "../Tsunami.h"
#include "../Stuff/Log.h"
#include "../Stuff/PerformanceMonitor.h"
#include "../lib/threads/Thread.h"
#include "DeviceManager.h"
#include "Device.h"
#include "OutputStream.h"

#include "../Audio/Source/AudioSource.h"

#ifdef DEVICE_PULSEAUDIO
#include <pulse/pulseaudio.h>
#endif

#ifdef DEVICE_PORTAUDIO
#include <portaudio.h>
#endif

//const int DEFAULT_BUFFER_SIZE = 131072;
const int DEFAULT_BUFFER_SIZE = 32768;
//const int DEFAULT_BUFFER_SIZE = 16384;

const float DEFAULT_UPDATE_DT = 0.050f;


const string OutputStream::MESSAGE_STATE_CHANGE = "StateChange";
const string OutputStream::MESSAGE_UPDATE = "Update";
const string OutputStream::MESSAGE_END_OF_STREAM = "EndOfStream";


#ifdef DEVICE_PULSEAUDIO

extern void pa_wait_op(pa_operation *op); // -> AudioOutput.cpp

bool pa_wait_stream_ready(pa_stream *s)
{
	//msg_write("wait stream ready");
	int n = 0;
	while (pa_stream_get_state(s) != PA_STREAM_READY){
		//pa_mainloop_iterate(m, 1, NULL);
		hui::Sleep(0.01f);
		n ++;
		if (n >= 200)
			return false;
		if (pa_stream_get_state(s) == PA_STREAM_FAILED)
			return false;
	}
	//msg_write("ok");
	return true;
}


void OutputStream::stream_request_callback(pa_stream *p, size_t nbytes, void *userdata)
{
	//printf("request %d\n", (int)nbytes);
	OutputStream *stream = (OutputStream*)userdata;

	void *data;
	int r = pa_stream_begin_write(p, &data, &nbytes);
	stream->testError("pa_stream_begin_write");
	//printf("%d  %p  %d\n", r, data, (int)nbytes);

	int done = 0;
	int frames = nbytes / 8;
	float *out = (float*)data;

	int available = stream->ring_buf.available();
	//printf("%d\n", available);
	if (stream->paused or (available < frames)){
		if (!stream->paused)
			printf("< underflow\n");
		// output silence...
		for (int i=0; i<frames; i++){
			*out ++ = 0;
			*out ++ = 0;
		}
	}else{
		for (int n=0; (n<2) and (done < frames); n++){
			AudioBuffer b;
			stream->ring_buf.readRef(b, frames - done);

			b.interleave(out, stream->device_manager->getOutputVolume() * stream->volume);
			out += b.length * 2;
			done += b.length;
			break;
		}
		done = frames;
	}

	pa_stream_write(p, data, nbytes, NULL, 0, (pa_seek_mode_t)PA_SEEK_RELATIVE);
	stream->testError("pa_stream_write");


	// read more?
	if ((available < stream->buffer_size) and (!stream->reading) and (!stream->read_more) and (!stream->read_end_of_data)){
		//printf("+\n");
		stream->read_more = true;
	}

	if (available <= frames and stream->read_end_of_data){
		//printf("end of data...\n");
		hui::RunLater(0.001f, std::bind(&OutputStream::onPlayedEndOfStream, stream)); // TODO prevent abort before playback really finished
	}
}

void OutputStream::stream_success_callback(pa_stream *s, int success, void *userdata)
{
	//msg_write("--success");
}

void OutputStream::stream_underflow_callback(pa_stream *s, void *userdata)
{
	OutputStream *stream = (OutputStream*)userdata;
	printf("pulse: underflow\n");
}

#endif

#ifdef DEVICE_PORTAUDIO

int OutputStream::stream_request_callback(const void *inputBuffer, void *outputBuffer,
                                          unsigned long frames,
                                          const PaStreamCallbackTimeInfo* timeInfo,
                                          PaStreamCallbackFlags statusFlags,
                                          void *userData)
{
	printf("request %d\n", (int)frames);
	OutputStream *stream = (OutputStream*)userData;

	float *out = (float*)outputBuffer;
	(void) inputBuffer; /* Prevent unused variable warning. */


	if (!stream->playing){
		//msg_error("not playing");
		return 1;
	}

	int done = 0;
	int available = stream->ring_buf.available();
	if ((stream->paused) or (available < frames)){
		if ((stream->playing) and (!stream->paused))
			printf("< underflow\n");
		// output silence...
		for (int i=0; i<frames; i++){
			*out ++ = 0;
			*out ++ = 0;
		}
	}else{
		for (int n=0; (n<2) and (done < frames); n++){
			AudioBuffer b;
			stream->ring_buf.readRef(b, frames - done);
			b.interleave(out, stream->device_manager->getOutputVolume() * stream->volume);
			out += b.length * 2;
			done += b.length;
			break;
		}
		done = frames;
		stream->cur_pos += done;
	}


	// read more?
	if ((available < stream->buffer_size) and (!stream->reading) and (!stream->read_more) and (!stream->end_of_data)){
		//printf("+\n");
		stream->read_more = true;
	}

	if (available <= frames and stream->end_of_data){
		//printf("end\n");
		HuiRunLaterM(0.001f, stream, &OutputStream::stop); // TODO prevent abort before playback really finished
	}
	return 0;
}

#endif

class StreamThread : public Thread
{
public:
	OutputStream *stream;
	int perf_channel;

	StreamThread(OutputStream *s)
	{
		stream = s;
		perf_channel = stream->perf_channel;
	}

	virtual void _cdecl onRun()
	{
		//printf("thread start\n");
		while(stream->keep_thread_running){
			if (stream->read_more){
				PerformanceMonitor::start_busy(perf_channel);
				stream->readStream();
				PerformanceMonitor::end_busy(perf_channel);
			}else{
				hui::Sleep(0.005f);
			}
		}
		//printf("thread end\n");
	}
};

OutputStream::OutputStream(AudioSource *r) :
	ring_buf(1048576)
{
	perf_channel = PerformanceMonitor::create_channel("out");
	source = r;

	fully_initialized = false;
	paused = false;
	volume = 1;
	read_more = false;
	reading = false;
	hui_runner_id = -1;
	keep_thread_running = true;

	device_manager = tsunami->device_manager;
	device = device_manager->chooseDevice(Device::TYPE_AUDIO_OUTPUT);

	data_samples = 0;
	buffer_size = DEFAULT_BUFFER_SIZE;
	update_dt = DEFAULT_UPDATE_DT;
	killed = false;
	thread = NULL;
#ifdef DEVICE_PULSEAUDIO
	_stream = NULL;
#endif
#ifdef DEVICE_PORTAUDIO
	_stream = NULL;
	err = paNoError;
#endif
	dev_sample_rate = -1;

	read_end_of_data = false;
	played_end_of_data = false;

	device_manager->addStream(this);
}

OutputStream::~OutputStream()
{
	//printf("del stream\n");
	if (hui_runner_id >= 0){
		hui::CancelRunner(hui_runner_id);
		hui_runner_id = -1;
	}

	kill_dev();

	device_manager->removeStream(this);
	killed = true;

	if (thread){
		keep_thread_running = false;
		thread->join();
		//thread->kill();
		delete(thread);
		thread = NULL;
	}
	PerformanceMonitor::delete_channel(perf_channel);
}

void OutputStream::__init__(AudioSource *r)
{
	new(this) OutputStream(r);
}

void OutputStream::__delete__()
{
	this->OutputStream::~OutputStream();
}

void OutputStream::create_dev()
{
	dev_sample_rate = source->getSampleRate();

#ifdef DEVICE_PULSEAUDIO
	if (_stream)
		return;

	pa_sample_spec ss;
	ss.rate = dev_sample_rate;
	ss.channels = 2;
	ss.format = PA_SAMPLE_FLOAT32LE;
	//ss.format = PA_SAMPLE_S16LE;
	_stream = pa_stream_new(device_manager->context, "stream", &ss, NULL);
	testError("pa_stream_new");

	pa_stream_set_write_callback(_stream, &stream_request_callback, this);
	pa_stream_set_underflow_callback(_stream, &stream_underflow_callback, this);
#endif
#ifdef DEVICE_PORTAUDIO
	if (_stream)
		return;
	err = Pa_OpenDefaultStream(&_stream, 0, 2, paFloat32, dev_sample_rate, 256,
	                           &stream_request_callback, this);
	testError("Pa_OpenDefaultStream");
#endif
}

void OutputStream::kill_dev()
{
#ifdef DEVICE_PULSEAUDIO

	if (_stream){
		pa_stream_disconnect(_stream);
		testError("pa_stream_disconnect");
		pa_stream_unref(_stream);
		testError("pa_stream_unref");
		_stream = NULL;
	}
#endif

#ifdef DEVICE_PORTAUDIO
	if (_stream){
		err = Pa_CloseStream(_stream);
		testError("Pa_CloseStream");
		_stream = NULL;
	}
#endif
}

void OutputStream::_stop()
{
	pause(true);

#if 0
	return;

	read_more = false;
	hui::CancelRunner(hui_runner_id);
	hui_runner_id = -1;

#ifdef DEVICE_PULSEAUDIO
	if (_stream){

		pa_operation *op = pa_stream_drain(_stream, NULL, NULL);
		testError("pa_stream_drain");
		pa_wait_op(op);

		kill_dev();
	}
#endif

#ifdef DEVICE_PORTAUDIO
	if (_stream){
		err = Pa_AbortStream(_stream);
		testError("Pa_AbortStream");

		kill_dev();
	}
#endif

	// stop thread
	thread->join();
	delete thread;
	thread = NULL;

	// clean up
	paused = false;
	read_end_of_data = false;
	played_end_of_data = false;
	ring_buf.clear();

	notify(MESSAGE_STATE_CHANGE);
#endif
}

void OutputStream::pause(bool _pause)
{
	//printf("stream pause %d\n", (int)_pause);
	if (!fully_initialized)
		start_first_time();

	paused = _pause;

	pa_operation *op = pa_stream_cork(_stream, paused, NULL, NULL);
	testError("pa_stream_cork");
	pa_wait_op(op);

	notify(MESSAGE_STATE_CHANGE);
}

void OutputStream::readStream()
{
	reading = true;
	read_more = false;

	int size = 0;
	AudioBuffer b;
	b.resize(buffer_size);

	// read data
	size = source->read(b);

	// out of data?
	if (size == source->END_OF_STREAM){
		read_end_of_data = true;
		reading = false;
		return;
	}

	// add to queue
	ring_buf.write(b);

	reading = false;
}

void OutputStream::setSource(AudioSource *r)
{
	source = r;
}

void OutputStream::setDevice(Device *d)
{
	device = d;
}

void OutputStream::_play()
{
	pause(false);
}

void OutputStream::start_first_time()
{
	//printf("stream start first\n");
	read_end_of_data = false;
	played_end_of_data = false;
	reading = false;

	// we need some data in the buffer...
	readStream();


	thread = new StreamThread(this);
	thread->run();


#ifdef DEVICE_PULSEAUDIO
	create_dev();
	if (!_stream)
		return;

	pa_buffer_attr attr_out;
	attr_out.fragsize = 1024;//-1;//512;
	attr_out.maxlength = 4096;
	attr_out.minreq = -1;
	attr_out.tlength = -1;
	attr_out.prebuf = -1;

	const char *dev = NULL;
	if (!device->is_default())
		dev = device->internal_name.c_str();
	pa_stream_connect_playback(_stream, dev, &attr_out, (pa_stream_flags)0, NULL, NULL);
	testError("pa_stream_connect_playback");


	if (!pa_wait_stream_ready(_stream)){
		msg_write("retry");

		// retry with default device
		pa_stream_connect_playback(_stream, NULL, &attr_out, (pa_stream_flags)0, NULL, NULL);
		testError("pa_stream_connect_playback");

		if (!pa_wait_stream_ready(_stream)){
			// still no luck... give up
			msg_write("aaaaa");
			tsunami->log->error("pa_wait_for_stream_ready");
			_stop();
			return;
		}
	}

	//stream_request_callback(_stream, ring_buf.available(), this);

	pa_operation *op = pa_stream_trigger(_stream, &stream_success_callback, NULL);
	testError("pa_stream_trigger");
	pa_wait_op(op);
#endif

#ifdef DEVICE_PORTAUDIO
	Pa_StartStream(_stream);
#endif

	fully_initialized = true;
	hui_runner_id = hui::RunRepeated(update_dt, std::bind(&OutputStream::update, this));

	notify(MESSAGE_STATE_CHANGE);
}

bool OutputStream::isPaused()
{
	return paused;
}

int OutputStream::getState()
{
	if (paused)
		return STATE_PAUSED;
	return STATE_PLAYING;
	//return STATE_STOPPED;
}

int OutputStream::getPos(int read_pos)
{
	return read_pos - ring_buf.available();
}

float OutputStream::getVolume()
{
	return volume;
}

void OutputStream::setVolume(float _volume)
{
	volume = _volume;
	notify(MESSAGE_STATE_CHANGE);
}

float OutputStream::getSampleRate()
{
	return source->getSampleRate();
}

void OutputStream::getSomeSamples(AudioBuffer &buf, int num_samples)
{
	ring_buf.peekRef(buf, num_samples);
}

bool OutputStream::testError(const string &msg)
{
#ifdef DEVICE_PULSEAUDIO
	int e = pa_context_errno(device_manager->context);
	if (e != 0){
		msg_error(msg + ": " + pa_strerror(e));
		return true;
	}
#endif
#ifdef DEVICE_PORTAUDIO
	if (err != paNoError){
		msg_error(Pa_GetErrorText(err));
		return true;
	}
#endif
	return false;
}

void OutputStream::update()
{
	testError("idle");

	if (!paused)
		notify(MESSAGE_UPDATE);
}

void OutputStream::onPlayedEndOfStream()
{
	//printf("stream end\n");
	notify(MESSAGE_END_OF_STREAM);
	pause(true);
}
