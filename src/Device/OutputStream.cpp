/*
 * OutputStream.cpp
 *
 *  Created on: 01.11.2014
 *      Author: michi
 */

#include "../Tsunami.h"
#include "../Audio/Renderer/AudioRenderer.h"
#include "../Stuff/Log.h"
#include <pulse/pulseaudio.h>
#include "../lib/threads/Thread.h"
#include "DeviceManager.h"
#include "Device.h"
#include "OutputStream.h"

//const int DEFAULT_BUFFER_SIZE = 131072;
const int DEFAULT_BUFFER_SIZE = 32768;
//const int DEFAULT_BUFFER_SIZE = 16384;

const float DEFAULT_UPDATE_DT = 0.050f;


const string OutputStream::MESSAGE_STATE_CHANGE = "StateChange";
const string OutputStream::MESSAGE_UPDATE = "Update";



extern void pa_wait_op(pa_operation *op); // -> AudioOutput.cpp

bool pa_wait_stream_ready(pa_stream *s)
{
	//msg_write("wait stream ready");
	int n = 0;
	while (pa_stream_get_state(s) != PA_STREAM_READY){
		//pa_mainloop_iterate(m, 1, NULL);
		HuiSleep(0.01f);
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
	stream->testError("begin write");
	//printf("%d  %p  %d\n", r, data, (int)nbytes);

	if (!stream->playing){
		//msg_error("not playing");
		pa_stream_cancel_write(p);
		stream->testError("cancel write");
		return;
	}

	int done = 0;
	int frames = nbytes / 8;
	float *out = (float*)data;

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
			BufferBox b;
			stream->ring_buf.readRef(b, frames - done);
			b.interleave(out, stream->device_manager->getOutputVolume() * stream->volume);
			out += b.length * 2;
			done += b.length;
			break;
		}
		done = frames;
		stream->cur_pos += done;
	}

	pa_stream_write(p, data, nbytes, NULL, 0, (pa_seek_mode_t)PA_SEEK_RELATIVE);
	stream->testError("write");


	// read more?
	if ((available < stream->buffer_size) and (!stream->reading) and (!stream->read_more) and (!stream->end_of_data)){
		//printf("+\n");
		stream->read_more = true;
	}

	if (available <= frames and stream->end_of_data){
		//printf("end\n");
		HuiRunLaterM(0.001f, stream, &OutputStream::stop); // TODO prevent abort before playback really finished
	}
}

void OutputStream::stream_success_callback(pa_stream *s, int success, void *userdata)
{
	//msg_write("--success");
}

void OutputStream::stream_underflow_callback(pa_stream *s, void *userdata)
{
	OutputStream *stream = (OutputStream*)userdata;
	if (stream->playing)
		printf("pulse: underflow\n");
}

class StreamThread : public Thread
{
public:
	OutputStream *stream;
	HuiTimer timer;
	float t_idle;

	StreamThread(OutputStream *s)
	{
		stream = s;
		t_idle = 0;
	}

	virtual void onRun()
	{
		timer.reset();
		//msg_write("thread run");
		while(stream->playing){
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
		//msg_write("thread done...");
	}
};

OutputStream::OutputStream(AudioRenderer *r) :
	PeakMeterSource("OutputStream"),
	ring_buf(1048576)
{
	renderer = r;

	playing = false;
	paused = false;
	volume = 1;
	read_more = false;
	reading = false;
	hui_runner_id = -1;

	device_manager = tsunami->device_manager;
	device = device_manager->chooseDevice(Device::TYPE_AUDIO_OUTPUT);

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

	device_manager->addStream(this);
}

OutputStream::~OutputStream()
{
	kill();
}

void OutputStream::__init__(AudioRenderer *r)
{
	new(this) OutputStream(r);
}

void OutputStream::__delete__()
{
	this->OutputStream::~OutputStream();
}

void OutputStream::create_dev()
{
	if (_stream)
		return;

	dev_sample_rate = renderer->getSampleRate();

	pa_sample_spec ss;
	ss.rate = dev_sample_rate;
	ss.channels = 2;
	ss.format = PA_SAMPLE_FLOAT32LE;
	//ss.format = PA_SAMPLE_S16LE;
	_stream = pa_stream_new(device_manager->context, "stream", &ss, NULL);
	testError("stream new");

	pa_stream_set_write_callback(_stream, &stream_request_callback, this);
	pa_stream_set_underflow_callback(_stream, &stream_underflow_callback, this);
}

void OutputStream::kill_dev()
{
	msg_db_f("Stream.kill_dev", 1);

	if (_stream){
		stop();

		/*pa_stream_unref(_stream);
		testError("unref");*/
		_stream = NULL;
	}
}

// only used for clean up
void OutputStream::kill()
{
	msg_db_f("Stream.kill", 1);

	if (_stream)
		kill_dev();

	device_manager->removeStream(this);
	killed = true;

	if (thread){
		thread->kill();
		delete(thread);
		thread = NULL;
	}
}

void OutputStream::stop()
{
	if (!playing)
		return;
	msg_db_f("Stream.stop", 1);

	playing = false;
	read_more = false;
	HuiCancelRunner(hui_runner_id);
	hui_runner_id = -1;

	if (_stream){

		pa_operation *op = pa_stream_drain(_stream, NULL, NULL);
		testError("drain");
		pa_wait_op(op);

		pa_stream_disconnect(_stream);
		testError("disconnect");
		pa_stream_unref(_stream);
		testError("unref");
		_stream = NULL;

		kill_dev();
	}

	// stop thread
	thread->join();
	delete thread;
	thread = NULL;

	// clean up
	playing = false;
	paused = false;
	end_of_data = false;
	ring_buf.clear();

	notify(MESSAGE_STATE_CHANGE);
}

void OutputStream::pause()
{
	if (!playing)
		return;

	paused = !paused;
	notify(MESSAGE_STATE_CHANGE);
}

void OutputStream::stream()
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

	// update pos
	Range rr = renderer->range();
	cur_pos = renderer->getPos() - rr.offset - ring_buf.available();
	if (rr.length > 0)
		cur_pos = rr.offset + (cur_pos + rr.length) % rr.length;

	reading = false;
}

void OutputStream::setSource(AudioRenderer *r)
{
	msg_db_f("Stream.setSource", 1);

	if (playing)
		stop();

	renderer = r;
}

void OutputStream::setDevice(Device *d)
{
	device = d;
}

void OutputStream::play()
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



	end_of_data = false;
	reading = false;


	playing = true;
	paused = false;
	cur_pos = renderer->range().offset;

	renderer->reset();
	stream();


	//if (!thread){
		thread = new StreamThread(this);
		thread->run();
	//}


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
	testError("connect");


	if (!pa_wait_stream_ready(_stream)){
		tsunami->log->error("pa_wait_for_stream_ready");
		stop();
		return;
	}

	//stream_request_callback(_stream, ring_buf.available(), this);

	pa_operation *op = pa_stream_trigger(_stream, &stream_success_callback, NULL);
	testError("trigger");
	pa_wait_op(op);

	hui_runner_id = HuiRunRepeatedM(update_dt, this, &OutputStream::update);

	notify(MESSAGE_STATE_CHANGE);
}

bool OutputStream::isPlaying()
{
	return playing;
}

int OutputStream::getState()
{
	if (playing and paused)
		return STATE_PAUSED;
	if (playing)
		return STATE_PLAYING;
	return STATE_STOPPED;
}

int OutputStream::getPos()
{
	int pos;
	if (getPosSafe(pos))
		return pos;
	return 0;
}

bool OutputStream::getPosSafe(int &pos)
{
	if (!playing)
		return false;

	pos = cur_pos;

	// translation
	Range r = renderer->range();
	if (r.length > 0)
		pos = cur_pos; //r.offset + ((cur_pos + renderer->offset()) %r.num);
	return true;
}

void OutputStream::seek(int pos)
{
	renderer->seek(pos);
	play();
	cur_pos = pos;
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
	return renderer->getSampleRate();
}

void OutputStream::getSomeSamples(BufferBox &buf, int num_samples)
{
	if (!playing)
		return;

	ring_buf.peekRef(buf, num_samples);
}

bool OutputStream::testError(const string &msg)
{
	int e = pa_context_errno(device_manager->context);
	if (e != 0)
		msg_error(msg + ": " + pa_strerror(e));
	return (e != 0);
}

void OutputStream::update()
{
	msg_db_f("Stream.update", 1);
	testError("idle");
	if (!playing)
		return;

	if (!paused)
		notify(MESSAGE_UPDATE);
}


