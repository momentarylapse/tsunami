/*
 * AudioOutput.cpp
 *
 *  Created on: 01.11.2014
 *      Author: michi
 */

#include "AudioOutput.h"

#include "../Session.h"
#include "../Data/base.h"
#include "../Module/Port/Port.h"
#include "../Device/Device.h"
#include "../Device/DeviceManager.h"
#include "../lib/kaba/syntax/Class.h"
#include "../Plugins/PluginManager.h"

namespace Kaba{
	VirtualTable* get_vtable(const VirtualBase *p);
}

#if HAS_LIB_PULSEAUDIO
#include <pulse/pulseaudio.h>
#endif

#if HAS_LIB_PORTAUDIO
#include <portaudio.h>
#endif

const int AudioOutput::DEFAULT_PREBUFFER_SIZE = 4096;


extern void require_main_thread(const string &msg); // hui

#if HAS_LIB_PULSEAUDIO

extern void pulse_wait_op(Session*, pa_operation*); // -> DeviceManager.cpp
extern void pulse_ignore_op(Session*, pa_operation*);

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


void AudioOutput::pulse_stream_request_callback(pa_stream *p, size_t nbytes, void *userdata)
{
//	printf("output request %d\n", (int)nbytes);
	AudioOutput *stream = (AudioOutput*)userdata;

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

void AudioOutput::pulse_stream_success_callback(pa_stream *s, int success, void *userdata)
{
	//msg_write("--success");
}

void AudioOutput::pulse_stream_underflow_callback(pa_stream *s, void *userdata)
{
	AudioOutput *stream = (AudioOutput*)userdata;
	//stream->session->w("pulse: underflow\n");
	printf("pulse: underflow\n");
}

#endif

bool AudioOutput::feed_stream_output(int frames_request, float *out)
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
		b.interleave(out, device_manager->get_output_volume() * config.volume);
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

int AudioOutput::portaudio_stream_request_callback(const void *inputBuffer, void *outputBuffer,
                                                    unsigned long frames,
                                                    const PaStreamCallbackTimeInfo* timeInfo,
                                                    PaStreamCallbackFlags statusFlags,
                                                    void *userData)
{
//	printf("request %d\n", (int)frames);
	AudioOutput *stream = (AudioOutput*)userData;

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


void AudioOutput::Config::reset()
{
	device = _module->session->device_manager->choose_device(DeviceType::AUDIO_OUTPUT);
	volume = 1;
}

string AudioOutput::Config::auto_conf(const string &name) const
{
	if (name == "volume")
		return "0:1:0.1:100:%";
	if (name == "device")
		return "audio:output";
	return "";
}


AudioOutput::AudioOutput(Session *_session) :
	Module(ModuleType::STREAM, "AudioOutput"),
	ring_buf(1048576)
{
	state = State::NO_DEVICE;
	set_session_etc(_session, "AudioOutput");
	source = nullptr;

	port_in.add(InPortDescription(SignalType::AUDIO, &source, "in"));

	config.volume = 1;
	prebuffer_size = hui::Config.get_int("Output.BufferSize", DEFAULT_PREBUFFER_SIZE);

	auto *device_pointer_class = session->plugin_manager->get_class("Device*");
	device_manager = session->device_manager;
	auto _class = session->plugin_manager->get_class("AudioOutputConfig");//new Kaba::Class("Config", sizeof(config), nullptr, nullptr);
	if (_class->elements.num == 0){
		_class->elements.add(Kaba::ClassElement("volume", Kaba::TypeFloat32, offsetof(Config, volume)));
		_class->elements.add(Kaba::ClassElement("device", device_pointer_class, offsetof(Config, device)));
		_class->_vtable_location_target_ = Kaba::get_vtable(&config);
	}
	config._class = _class;

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

AudioOutput::~AudioOutput()
{
	_pause();
	_kill_dev();

	device_manager->remove_stream(this);
}

void AudioOutput::__init__(Session *s)
{
	new(this) AudioOutput(s);
}

void AudioOutput::__delete__()
{
	this->AudioOutput::~AudioOutput();
}

void AudioOutput::_create_dev()
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
		if (!config.device->is_default())
			dev = config.device->internal_name.c_str();
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


		if (config.device->is_default()){
			PaError err = Pa_OpenDefaultStream(&portaudio_stream, 0, 2, paFloat32, dev_sample_rate, paFramesPerBufferUnspecified,//256,
					&portaudio_stream_request_callback, this);
			_portaudio_test_error(err, "Pa_OpenDefaultStream");
		}else{
			PaStreamParameters params;
			params.channelCount = 2;
			params.sampleFormat = paFloat32;
			params.device = config.device->index_in_lib;
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

void AudioOutput::_kill_dev()
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




void AudioOutput::stop()
{
	require_main_thread("out.stop");
	_pause();
}

void AudioOutput::_pause()
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


	state = State::PAUSED;
	notify(MESSAGE_STATE_CHANGE);
}

void AudioOutput::_unpause()
{
	if (state != State::PAUSED)
		return;
	session->debug("out", "unpause");

	read_end_of_stream = false;
	played_end_of_stream = false;

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

int AudioOutput::_read_stream(int buffer_size)
{
	if (!source)
		return 0;

	int size = 0;
	AudioBuffer b;
	ring_buf.write_ref(b, buffer_size);

	// read data
	size = source->read_audio(b);

	if (size == source->NOT_ENOUGH_DATA){
		//printf(" -> no data\n");
		// keep trying...
		ring_buf.write_ref_cancel(b);
		return size;
	}

	// out of data?
	if (size == source->END_OF_STREAM){
		//printf(" -> end  STREAM\n");
		read_end_of_stream = true;
		hui::RunLater(0.001f,  [=]{ on_read_end_of_stream(); });
		ring_buf.write_ref_cancel(b);
		return size;
	}

	// add to queue
	b.length = size;
	ring_buf.write_ref_done(b);
	buffer_is_cleared = false;
//	printf(" -> %d of %d\n", size, buffer_size);
	return size;
}

void AudioOutput::set_device(Device *d)
{
	config.device = d;
}

void AudioOutput::start()
{
	require_main_thread("out.start");
	if (state == State::NO_DEVICE)
		_create_dev();

	if (state == State::DEVICE_WITHOUT_DATA)
		_fill_prebuffer();

	_unpause();
}

void AudioOutput::_fill_prebuffer()
{
	if (state != State::DEVICE_WITHOUT_DATA)
		return;
	session->debug("out", "prebuf");

	// we need some data in the buffer...
	_read_stream(prebuffer_size);

#if HAS_LIB_PULSEAUDIO
	if (device_manager->audio_api == DeviceManager::ApiType::PULSE){
		if (!pulse_stream)
			return;

		pa_operation *op = pa_stream_prebuf(pulse_stream, &pulse_stream_success_callback, nullptr);
		_pulse_test_error("pa_stream_prebuf");
		pulse_ignore_op(session, op);

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

int AudioOutput::get_available()
{
	return ring_buf.available();
}

float AudioOutput::get_volume()
{
	return config.volume;
}

void AudioOutput::set_volume(float _volume)
{
	config.volume = _volume;
	notify(MESSAGE_STATE_CHANGE);
}

void AudioOutput::set_prebuffer_size(int size)
{
	prebuffer_size = size;
}

#if HAS_LIB_PULSEAUDIO
bool AudioOutput::_pulse_test_error(const string &msg)
{
	int e = pa_context_errno(device_manager->pulse_context);
	if (e != 0){
		session->e("AudioOutput: " + msg + ": " + pa_strerror(e));
		return true;
	}
	return false;
}
#endif

#if HAS_LIB_PORTAUDIO
bool AudioOutput::_portaudio_test_error(PaError err, const string &msg)
{
	if (err != paNoError){
		session->e("AudioOutput: " + msg + ": " + Pa_GetErrorText(err));
		return true;
	}
	return false;
}
#endif

void AudioOutput::on_played_end_of_stream()
{
	//printf("---------ON PLAY END OF STREAM\n");
	notify(MESSAGE_PLAY_END_OF_STREAM);
}

void AudioOutput::on_read_end_of_stream()
{
	//printf("---------ON READ END OF STREAM\n");
	notify(MESSAGE_READ_END_OF_STREAM);
}

void AudioOutput::reset_state()
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
			if (pulse_stream){
				pa_operation *op = pa_stream_flush(pulse_stream, nullptr, nullptr);
				_pulse_test_error("pa_stream_flush");
				pulse_ignore_op(session, op);
			}
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

int AudioOutput::command(ModuleCommand cmd, int param)
{
	if (cmd == ModuleCommand::START){
		start();
		return 0;
	}else if (cmd == ModuleCommand::STOP){
		stop();
		return 0;
	}else if (cmd == ModuleCommand::PREPARE_START){
		if (state == State::NO_DEVICE)
			_create_dev();
		_fill_prebuffer();
		return 0;
	}else if (cmd == ModuleCommand::SUCK){
		if (ring_buf.available() >= prebuffer_size)
			return 0;
		return _read_stream(param);
	}
	return COMMAND_NOT_HANDLED;
}

bool AudioOutput::is_playing()
{
	return (state == State::PLAYING) or (state == State::PAUSED);
}

ModuleConfiguration *AudioOutput::get_config() const
{
	return (ModuleConfiguration*)&config;
}
