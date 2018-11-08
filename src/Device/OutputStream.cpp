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

#include "../Module/Port/AudioPort.h"

#if HAS_LIB_PULSEAUDIO
#include <pulse/pulseaudio.h>
#endif

#if HAS_LIB_PORTAUDIO
#include <portaudio.h>
#endif

//const int DEFAULT_BUFFER_SIZE = 131072;
//const int DEFAULT_BUFFER_SIZE = 32768;
//const int DEFAULT_BUFFER_SIZE = 16384;
const int DEFAULT_BUFFER_SIZE = 4096;

const float DEFAULT_UPDATE_DT = 0.050f;


const string OutputStream::MESSAGE_STATE_CHANGE = "StateChange";
const string OutputStream::MESSAGE_UPDATE = "Update";
const string OutputStream::MESSAGE_READ_END_OF_STREAM = "ReadEndOfStream";
const string OutputStream::MESSAGE_PLAY_END_OF_STREAM = "PlayEndOfStream";


#if HAS_LIB_PULSEAUDIO

extern void pa_wait_op(Session*, pa_operation*); // -> AudioOutput.cpp

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
		hui::RunLater(0.001f, std::bind(&OutputStream::on_played_end_of_stream, stream)); // TODO prevent abort before playback really finished
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
	if (paused){
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
		hui::RunLater(0.001f, std::bind(&OutputStream::on_played_end_of_stream, stream)); // TODO prevent abort before playback really finished
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

extern bool ugly_hack_slow;

OutputStream::OutputStream(Session *_session) :
	Module(ModuleType::OUTPUT_STREAM_AUDIO),
	ring_buf(1048576)
{
//	printf("output new\n");
	perf_channel = PerformanceMonitor::create_channel("out");
	set_session_etc(_session, "", nullptr);
	source = nullptr;

	port_in.add(InPortDescription(SignalType::AUDIO, (Port**)&source, "in"));

	fully_initialized = false;
	paused = false;
	volume = 1;
	hui_runner_id = -1;

	device_manager = session->device_manager;
	device = device_manager->choose_device(DeviceType::AUDIO_OUTPUT);

	data_samples = 0;
	buffer_size = DEFAULT_BUFFER_SIZE;
	update_dt = DEFAULT_UPDATE_DT;
	if (ugly_hack_slow)
		update_dt *= 10;
	killed = false;
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
	if (hui_runner_id >= 0){
		hui::CancelRunner(hui_runner_id);
		hui_runner_id = -1;
	}

	_kill_dev();

	device_manager->remove_stream(this);
	killed = true;

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
}

void OutputStream::_kill_dev()
{
//	printf("output kill dev\n");
	if (!paused)
		_pause();

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
}




void OutputStream::stop()
{
	_pause();
	if (thread){
		thread->keep_running = false;
		thread->join();
		delete thread;
		thread = nullptr;
	}

	if (source)
		source->reset();

	clear_buffer();
}

void OutputStream::_pause()
{
	if (!fully_initialized)
		return;
	if (is_paused())
		return;
//	printf("pause...");

	hui::CancelRunner(hui_runner_id);
	hui_runner_id = -1;
	paused = true;

#if HAS_LIB_PULSEAUDIO
	if (pulse_stream){
		pa_operation *op = pa_stream_cork(pulse_stream, true, nullptr, nullptr);
		_pulse_test_error("pa_stream_cork");
		pa_wait_op(session, op);
	}
#endif
#if HAS_LIB_PORTAUDIO
	if (portaudio_stream){
		PaError err = Pa_StopStream(portaudio_stream);
		_portaudio_test_error(err, "Pa_StopStream");
	}
#endif
//	printf("ok\n");

	Observable<VirtualBase>::notify(MESSAGE_STATE_CHANGE);
}

void OutputStream::_unpause()
{
	if (!fully_initialized)
		return;
	if (!is_paused())
		return;
//	printf("unpause...");

	read_end_of_stream = false;
	played_end_of_stream = false;
	paused = false;

	if (!thread){
		thread = new StreamThread(this);
		thread->run();
	}

#if HAS_LIB_PULSEAUDIO
	if (pulse_stream){
		pa_operation *op = pa_stream_cork(pulse_stream, false, nullptr, nullptr);
		_pulse_test_error("pa_stream_cork");
		pa_wait_op(session, op);
	}
#endif
#if HAS_LIB_PORTAUDIO
	if (portaudio_stream){
		PaError err = Pa_StartStream(portaudio_stream);
		_portaudio_test_error(err, "Pa_StartStream");
	}
#endif
//	printf("ok\n");

	hui_runner_id = hui::RunRepeated(update_dt, std::bind(&OutputStream::update, this));
	Observable<VirtualBase>::notify(MESSAGE_STATE_CHANGE);
}

void OutputStream::pause(bool __pause)
{
	if (paused == __pause)
		return;

	if (__pause)
		_pause();
	else
		_unpause();
}

void OutputStream::_read_stream()
{
	if (!source)
		return;
	//printf("read stream\n");

	int size = 0;
	AudioBuffer b;
	ring_buf.write_ref(b, buffer_size);

	// read data
	size = source->read(b);

	if (size == source->NOT_ENOUGH_DATA){
		//printf(" -> no data\n");
		// keep trying...
		return;
	}

	// out of data?
	if (size == source->END_OF_STREAM){
		//printf(" -> end  STREAM\n");
		read_end_of_stream = true;
		hui::RunLater(0.001f,  std::bind(&OutputStream::on_read_end_of_stream, this));
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

void OutputStream::set_update_dt(float dt)
{
	update_dt = dt;
}

void OutputStream::play()
{
	if (fully_initialized)
		_unpause();
	else
		_start_first_time();
}

void OutputStream::_start_first_time()
{
//	printf("stream start first\n");
	read_end_of_stream = false;
	played_end_of_stream = false;

	// we need some data in the buffer...
	_read_stream();


	thread = new StreamThread(this);
	thread->run();

	_create_dev();

#if HAS_LIB_PULSEAUDIO
	if (device_manager->audio_api == DeviceManager::ApiType::PULSE){
		if (!pulse_stream)
			return;

		pa_buffer_attr attr_out;
		attr_out.fragsize = 1024;//-1;//512;
		attr_out.maxlength = 4096;
		attr_out.minreq = -1;
		attr_out.tlength = -1;
		attr_out.prebuf = -1;

		const char *dev = nullptr;
		if (!device->is_default())
			dev = device->internal_name.c_str();
		pa_stream_connect_playback(pulse_stream, dev, &attr_out, (pa_stream_flags)0, nullptr, nullptr);
		_pulse_test_error("pa_stream_connect_playback");


		if (!pa_wait_stream_ready(pulse_stream)){
			msg_write("retry");

			// retry with default device
			pa_stream_connect_playback(pulse_stream, nullptr, &attr_out, (pa_stream_flags)0, nullptr, nullptr);
			_pulse_test_error("pa_stream_connect_playback");

			if (!pa_wait_stream_ready(pulse_stream)){
				// still no luck... give up
				msg_write("aaaaa");
				session->e("pa_wait_for_stream_ready");
				stop();
				return;
			}
		}

		//stream_request_callback(_stream, ring_buf.available(), this);

		pa_operation *op = pa_stream_trigger(pulse_stream, &pulse_stream_success_callback, nullptr);
		_pulse_test_error("pa_stream_trigger");
		pa_wait_op(session, op);
	}
#endif

#if HAS_LIB_PORTAUDIO
	if (device_manager->audio_api == DeviceManager::ApiType::PORTAUDIO){
		if (!portaudio_stream)
			return;
		PaError err = Pa_StartStream(portaudio_stream);
		_portaudio_test_error(err, "Pa_StartStream");
	}
#endif

	fully_initialized = true;
	hui_runner_id = hui::RunRepeated(update_dt, std::bind(&OutputStream::update, this));

	Observable<VirtualBase>::notify(MESSAGE_STATE_CHANGE);
}

bool OutputStream::is_paused()
{
	return paused;
}

int OutputStream::get_pos()
{
	if (!source)
		return 0;
	return source->get_pos(- ring_buf.available());
}

float OutputStream::get_volume()
{
	return volume;
}

void OutputStream::set_volume(float _volume)
{
	volume = _volume;
	Observable<VirtualBase>::notify(MESSAGE_STATE_CHANGE);
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

void OutputStream::update()
{
//	testError("idle");

	if (!paused)
		Observable<VirtualBase>::notify(MESSAGE_UPDATE);
}

void OutputStream::on_played_end_of_stream()
{
	//printf("---------ON PLAY END OF STREAM\n");
	//pause(true);

	Observable<VirtualBase>::notify(MESSAGE_PLAY_END_OF_STREAM);
}

void OutputStream::on_read_end_of_stream()
{
	//printf("---------ON READ END OF STREAM\n");
	read_end_of_stream = true;

/*#ifdef DEVICE_PULSEAUDIO
	pa_operation *op = pa_stream_drain(_stream, NULL, NULL);
	testError("pa_stream_drain");
	pa_wait_op(op);
#endif*/
	// should drain...and use pa_stream_set_state_callback for notification

	Observable<VirtualBase>::notify(MESSAGE_READ_END_OF_STREAM);
}

void OutputStream::clear_buffer()
{
	ring_buf.clear();
	buffer_is_cleared = true;
}
