/*
 * OutputStream.cpp
 *
 *  Created on: 01.11.2014
 *      Author: michi
 */

#include "../Session.h"
#include "../Stuff/PerformanceMonitor.h"
#include "../lib/threads/Thread.h"
#include "../Data/base.h"
#include "DeviceManager.h"
#include "Device.h"
#include "OutputStream.h"
#include "../Module/Port/Port.h"

#if HAS_LIB_PULSEAUDIO
#include <pulse/pulseaudio.h>
#endif

#if HAS_LIB_PORTAUDIO
#include <portaudio.h>
#endif

const int DEFAULT_BUFFER_SIZE = 4096;



#if HAS_LIB_PULSEAUDIO

extern void pulse_wait_op(Session*, pa_operation*); // -> DeviceManager.cpp
extern void pulse_ignore_op(Session*, pa_operation*);
extern void require_main_thread(const string &msg); // hui

bool pulse_wait_stream_ready(pa_stream *s)
{
	//msg_write("wait stream ready");
	int n = 0;
	while (pa_stream_get_state(s) != PA_STREAM_READY){
		//pa_mainloop_iterate(m, 1, NULL);
		hui::Sleep(0.01f);
		n ++;
		if (n >= 1000)
			return false;
		if (pa_stream_get_state(s) == PA_STREAM_FAILED)
			return false;
	}
	//msg_write("ok");
	return true;
}


void OutputStream::pulse_stream_request_callback(pa_stream *p, size_t nbytes, void *userdata)
{
//	printf("output request %d\n", (int)nbytes);
	OutputStream *stream = (OutputStream*)userdata;

	void *data;
	int r = pa_stream_begin_write(p, &data, &nbytes);
	stream->_pulse_test_error("pa_stream_begin_write");
	//printf("%d  %p  %d\n", r, data, (int)nbytes);

	int frames = nbytes / 8;
	float *out = (float*)data;

	bool out_of_data = stream->feed_stream_output(frames, out);

	pa_stream_write(p, data, nbytes, nullptr, 0, (pa_seek_mode_t)PA_SEEK_RELATIVE);
	stream->_pulse_test_error("pa_stream_write");

	if (out_of_data and stream->read_end_of_stream and !stream->played_end_of_stream){
		//printf("end of data...\n");
		stream->played_end_of_stream = true;
		hui::RunLater(0.001f, [stream]{ stream->on_played_end_of_stream(); }); // TODO prevent abort before playback really finished
	}
}

void OutputStream::pulse_stream_success_callback(pa_stream *s, int success, void *userdata)
{
	//msg_write("--success");
}

void OutputStream::pulse_stream_underflow_callback(pa_stream *s, void *userdata)
{
	OutputStream *stream = (OutputStream*)userdata;
	//stream->session->w("pulse: underflow\n");
	printf("pulse: underflow\n");
}

#endif

bool OutputStream::feed_stream_output(int frames_request, float *out)
{
	if (state != State::PLAYING){
		// output silence...
		memset(out, 0, frames_request * 8);
		return false;
	}


	int done = 0;

	int available = ring_buf.available();
	int frames = min(frames_request, available);


//	printf("av=%d r=%d reos=%d\n", available, stream->reading.load(), stream->read_end_of_stream.load());


//	printf("  j\n");
	for (int n=0; (n<2) and (done < (int)frames); n++){
		AudioBuffer b;
		ring_buf.read_ref(b, frames - done);
		b.interleave(out, device_manager->get_output_volume() * volume);
		ring_buf.read_ref_done(b);
		out += b.length * 2;
		done += b.length;
	}
	done = frames;


	if (available < frames_request){
//		printf("  x\n");
		if (!read_end_of_stream and !buffer_is_cleared)
			printf("< underflow  %d < %d\n", available, frames_request);
		// output silence...
		for (int i=done; i<frames_request; i++){
			*out ++ = 0;
			*out ++ = 0;
		}
		return true;
	}

	return false;

}

#if HAS_LIB_PORTAUDIO

int OutputStream::portaudio_stream_request_callback(const void *inputBuffer, void *outputBuffer,
                                                    unsigned long frames,
                                                    const PaStreamCallbackTimeInfo* timeInfo,
                                                    PaStreamCallbackFlags statusFlags,
                                                    void *userData)
{
//	printf("request %d\n", (int)frames);
	OutputStream *stream = (OutputStream*)userData;

	float *out = (float*)outputBuffer;
	(void) inputBuffer; /* Prevent unused variable warning. */

	bool out_of_data = stream->feed_stream_output((int)frames, out);

	if (out_of_data and stream->read_end_of_stream and !stream->played_end_of_stream){
		//printf("XXX end of data...\n");
		stream->played_end_of_stream = true;
		hui::RunLater(0.001f, [stream]{ stream->on_played_end_of_stream(); }); // TODO prevent abort before playback really finished
		//printf("/XXX end of data...\n");
	}
	return 0;
}

#endif

class StreamThread : public Thread
{
public:
	OutputStream *stream;
	int perf_channel;
	bool keep_running = true;

	StreamThread(OutputStream *s)
	{
		stream = s;
		perf_channel = stream->perf_channel;
	}

	void on_run() override
	{
		//printf("thread start\n");
		while(keep_running){
			if (stream->read_end_of_stream){
				hui::Sleep(0.010f);
			}else{
				if (stream->ring_buf.available() <= stream->buffer_size){
					PerformanceMonitor::start_busy(perf_channel);
				//	printf("READ\n");
					stream->_read_stream();
					PerformanceMonitor::end_busy(perf_channel);
				}else{
					hui::Sleep(0.002f);
				}
			}
			Thread::cancelation_point();
		}
		//printf("thread end\n");
	}
};


OutputStream::OutputStream(Session *_session) :
	Module(ModuleType::STREAM, "AudioOutput"),
	ring_buf(1048576)
{
//	printf("output new\n");
	perf_channel = PerformanceMonitor::create_channel("out");
	state = State::NO_DEVICE;
	set_session_etc(_session, "", nullptr);
	source = nullptr;

	port_in.add(InPortDescription(SignalType::AUDIO, &source, "in"));

	volume = 1;

	device_manager = session->device_manager;
	device = device_manager->choose_device(DeviceType::AUDIO_OUTPUT);

	data_samples = 0;
	buffer_size = DEFAULT_BUFFER_SIZE;
	thread = nullptr;
#if HAS_LIB_PULSEAUDIO
	pulse_stream = nullptr;
#endif
#if HAS_LIB_PORTAUDIO
	portaudio_stream = nullptr;
#endif
	dev_sample_rate = -1;

	read_end_of_stream = false;
	played_end_of_stream = false;

	buffer_is_cleared = true;

	device_manager->add_stream(this);
}

OutputStream::~OutputStream()
{
	_pause();
	_kill_dev();

	device_manager->remove_stream(this);

	if (thread){
		thread->keep_running = false;
		thread->join();
		delete(thread); // automatic cancel
		thread = nullptr;
	}
	PerformanceMonitor::delete_channel(perf_channel);
}

void OutputStream::__init__(Session *s)
{
	new(this) OutputStream(s);
}

void OutputStream::__delete__()
{
	this->OutputStream::~OutputStream();
}

void OutputStream::_create_dev()
{
	if (state != State::NO_DEVICE)
		return;
	session->debug("out", "create device");
	dev_sample_rate = session->sample_rate();

#if HAS_LIB_PULSEAUDIO
	if (device_manager->audio_api == DeviceManager::ApiType::PULSE){
		pa_sample_spec ss;
		ss.rate = dev_sample_rate;
		ss.channels = 2;
		ss.format = PA_SAMPLE_FLOAT32LE;
		//ss.format = PA_SAMPLE_S16LE;
		pulse_stream = pa_stream_new(device_manager->pulse_context, "stream", &ss, nullptr);
		_pulse_test_error("pa_stream_new");

		pa_stream_set_write_callback(pulse_stream, &pulse_stream_request_callback, this);
		pa_stream_set_underflow_callback(pulse_stream, &pulse_stream_underflow_callback, this);



		pa_buffer_attr attr_out;
		attr_out.fragsize = 1024;//-1;//512;
		attr_out.maxlength = 4096;
		attr_out.minreq = -1;
		attr_out.tlength = -1;
		attr_out.prebuf = -1;

		const char *dev = nullptr;
		if (!device->is_default())
			dev = device->internal_name.c_str();
		pa_stream_connect_playback(pulse_stream, dev, &attr_out, PA_STREAM_START_CORKED, nullptr, nullptr);
		_pulse_test_error("pa_stream_connect_playback");


		if (!pulse_wait_stream_ready(pulse_stream)){
			session->w("retry");

			// retry with default device
			pa_stream_connect_playback(pulse_stream, nullptr, &attr_out, PA_STREAM_START_CORKED, nullptr, nullptr);
			_pulse_test_error("pa_stream_connect_playback");

			if (!pulse_wait_stream_ready(pulse_stream)){
				// still no luck... give up
				session->e("pulse_wait_stream_ready");
				stop();
				return;
			}
		}
	}
#endif

#if HAS_LIB_PORTAUDIO
	if (device_manager->audio_api == DeviceManager::ApiType::PORTAUDIO){


		if (device->is_default()){
			PaError err = Pa_OpenDefaultStream(&portaudio_stream, 0, 2, paFloat32, dev_sample_rate, paFramesPerBufferUnspecified,//256,
					&portaudio_stream_request_callback, this);
			_portaudio_test_error(err, "Pa_OpenDefaultStream");
		}else{
			PaStreamParameters params;
			params.channelCount = 2;
			params.sampleFormat = paFloat32;
			params.device = device->index_in_lib;
			params.hostApiSpecificStreamInfo = nullptr;
			params.suggestedLatency = 0;
			PaError err = Pa_OpenStream(&portaudio_stream, nullptr, &params, dev_sample_rate, paFramesPerBufferUnspecified,//256,
					paNoFlag, &portaudio_stream_request_callback, this);
			_portaudio_test_error(err, "Pa_OpenStream");
		}
	}
#endif
	state = State::DEVICE_WITHOUT_DATA;
}

void OutputStream::_kill_dev()
{
	if (state != State::DEVICE_WITHOUT_DATA and state != State::PAUSED)
		return;
	session->debug("out", "kill device");
#if HAS_LIB_PULSEAUDIO
	if (pulse_stream){
		pa_stream_disconnect(pulse_stream);
		_pulse_test_error("pa_stream_disconnect");
		pa_stream_unref(pulse_stream);
		_pulse_test_error("pa_stream_unref");
		pulse_stream = nullptr;
	}
#endif

#if HAS_LIB_PORTAUDIO
	if (portaudio_stream){
		PaError err = Pa_CloseStream(portaudio_stream);
		_portaudio_test_error(err, "Pa_CloseStream");
		portaudio_stream = nullptr;
	}
#endif
	state = State::NO_DEVICE;
}




void OutputStream::stop()
{
	require_main_thread("out.stop");
	_pause();
}

void OutputStream::_pause()
{
	if (state != State::PLAYING)
		return;
	session->debug("out", "pause");

#if HAS_LIB_PULSEAUDIO
	if (pulse_stream){
		pa_operation *op = pa_stream_cork(pulse_stream, true, nullptr, nullptr);
		_pulse_test_error("pa_stream_cork");
		pulse_ignore_op(session, op);
	}
#endif
#if HAS_LIB_PORTAUDIO
	if (portaudio_stream){
		//Pa_AbortStream Pa_StopStream
		PaError err = Pa_AbortStream(portaudio_stream);
		_portaudio_test_error(err, "Pa_AbortStream");
	}
#endif


	if (thread){
		thread->keep_running = false;
		thread->join();
		delete thread;
		thread = nullptr;
	}

	state = State::PAUSED;
	notify(MESSAGE_STATE_CHANGE);
}

void OutputStream::_unpause()
{
	if (state != State::PAUSED)
		return;
	session->debug("out", "unpause");

	read_end_of_stream = false;
	played_end_of_stream = false;

	if (!thread){
		thread = new StreamThread(this);
		thread->run();
	}

#if HAS_LIB_PULSEAUDIO
	if (pulse_stream){
		pa_operation *op = pa_stream_cork(pulse_stream, false, nullptr, nullptr);
		_pulse_test_error("pa_stream_cork");
		pulse_ignore_op(session, op);
	}
#endif
#if HAS_LIB_PORTAUDIO
	if (portaudio_stream){
		PaError err = Pa_StartStream(portaudio_stream);
		_portaudio_test_error(err, "Pa_StartStream");
	}
#endif

	state = State::PLAYING;
	notify(MESSAGE_STATE_CHANGE);
}

void OutputStream::_read_stream()
{
	if (!source)
		return;

	int size = 0;
	AudioBuffer b;
	ring_buf.write_ref(b, buffer_size);

	// read data
	size = source->read_audio(b);

	if (size == source->NOT_ENOUGH_DATA){
		//printf(" -> no data\n");
		// keep trying...
		ring_buf.write_ref_cancel(b);
		return;
	}

	// out of data?
	if (size == source->END_OF_STREAM){
		//printf(" -> end  STREAM\n");
		read_end_of_stream = true;
		hui::RunLater(0.001f,  [=]{ on_read_end_of_stream(); });
		ring_buf.write_ref_cancel(b);
		return;
	}

	// add to queue
	b.length = size;
	ring_buf.write_ref_done(b);
	buffer_is_cleared = false;
//	printf(" -> %d of %d\n", size, buffer_size);
}

void OutputStream::set_device(Device *d)
{
	device = d;
}

void OutputStream::start()
{
	require_main_thread("out.start");
	if (state == State::NO_DEVICE)
		_create_dev();

	if (state == State::DEVICE_WITHOUT_DATA)
		_fill_prebuffer();

	_unpause();
}

void OutputStream::_fill_prebuffer()
{
	if (state != State::DEVICE_WITHOUT_DATA)
		return;
	session->debug("out", "prebuf");

	// we need some data in the buffer...
	_read_stream();

	if (!thread){
		thread = new StreamThread(this);
		thread->run();
	}

#if HAS_LIB_PULSEAUDIO
	if (device_manager->audio_api == DeviceManager::ApiType::PULSE){
		if (!pulse_stream)
			return;

		pa_operation *op = pa_stream_prebuf(pulse_stream, &pulse_stream_success_callback, nullptr);
		_pulse_test_error("pa_stream_prebuf");
		pulse_wait_op(session, op);

		/*pa_operation *op = pa_stream_trigger(pulse_stream, &pulse_stream_success_callback, nullptr);
		_pulse_test_error("pa_stream_trigger");
		pa_wait_op(session, op);*/
	}
#endif

#if HAS_LIB_PORTAUDIO
	if (device_manager->audio_api == DeviceManager::ApiType::PORTAUDIO){
		if (!portaudio_stream)
			return;
		//PaError err = Pa_StartStream(portaudio_stream);
		//_portaudio_test_error(err, "Pa_StartStream");
	}
#endif

	state = State::PAUSED;
	notify(MESSAGE_STATE_CHANGE);
}

int OutputStream::get_available()
{
	return ring_buf.available();
}

float OutputStream::get_volume()
{
	return volume;
}

void OutputStream::set_volume(float _volume)
{
	volume = _volume;
	notify(MESSAGE_STATE_CHANGE);
}

#if HAS_LIB_PULSEAUDIO
bool OutputStream::_pulse_test_error(const string &msg)
{
	int e = pa_context_errno(device_manager->pulse_context);
	if (e != 0){
		session->e("OutputStream: " + msg + ": " + pa_strerror(e));
		return true;
	}
	return false;
}
#endif

#if HAS_LIB_PORTAUDIO
bool OutputStream::_portaudio_test_error(PaError err, const string &msg)
{
	if (err != paNoError){
		session->e("OutputStream: " + msg + ": " + Pa_GetErrorText(err));
		return true;
	}
	return false;
}
#endif

void OutputStream::on_played_end_of_stream()
{
	//printf("---------ON PLAY END OF STREAM\n");
	notify(MESSAGE_PLAY_END_OF_STREAM);
}

void OutputStream::on_read_end_of_stream()
{
	//printf("---------ON READ END OF STREAM\n");
	notify(MESSAGE_READ_END_OF_STREAM);
}

void OutputStream::reset_state()
{
	if (state == State::NO_DEVICE)
		return;
	if (state == State::PLAYING)
		_pause();
	require_main_thread("out.reset");
	if (state == State::PAUSED){
#if HAS_LIB_PULSEAUDIO
		if (device_manager->audio_api == DeviceManager::ApiType::PULSE){
			session->debug("out", "flush");
			if (pulse_stream)
				pa_stream_flush(pulse_stream, nullptr, nullptr);
			session->debug("out", "/flush");
		}
#endif
		state = State::DEVICE_WITHOUT_DATA;
	}

	ring_buf.clear();
	buffer_is_cleared = true;
	read_end_of_stream = false;
	played_end_of_stream = false;
}

int OutputStream::command(ModuleCommand cmd, int param)
{
	if (cmd == ModuleCommand::START){
		start();
	}else if (cmd == ModuleCommand::STOP){
		stop();
	}else if (cmd == ModuleCommand::PREPARE_START){
		if (state == State::NO_DEVICE)
			_create_dev();
		_fill_prebuffer();
	}
	return 0;
}

bool OutputStream::is_playing()
{
	return (state == State::PLAYING) or (state == State::PAUSED);
}
