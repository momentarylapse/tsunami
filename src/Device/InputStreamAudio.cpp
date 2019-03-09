/*
 * InputStreamAudio.cpp
 *
 *  Created on: 26.03.2012
 *      Author: michi
 */

#include "../lib/hui/hui.h"
#include "../Session.h"
#include "../View/AudioView.h"

#include "DeviceManager.h"
#include "Device.h"
#include "InputStreamAudio.h"
#include "OutputStream.h"
#include "../Stuff/BackupManager.h"
#include "../Data/base.h"

#if HAS_LIB_PULSEAUDIO
#include <pulse/pulseaudio.h>
#endif

#if HAS_LIB_PORTAUDIO
#include <portaudio.h>
#endif


float InputStreamAudio::playback_delay_const;


static const int DEFAULT_CHUNK_SIZE = 512;


#if HAS_LIB_PULSEAUDIO
extern void pa_wait_op(Session *session, pa_operation *op); // -> DeviceManager.cpp
extern bool pa_wait_stream_ready(pa_stream *s); // -> OutputStream.cpp


void InputStreamAudio::pulse_stream_request_callback(pa_stream *p, size_t nbytes, void *userdata)
{
	//printf("input request %d\n", (int)nbytes);
	InputStreamAudio *input = (InputStreamAudio*)userdata;

	const void *data;
	pa_stream_peek(p, &data, &nbytes);
	input->_pulse_test_error("pa_stream_peek");
	int frames = nbytes / sizeof(float) / input->num_channels;

	if (data){
		if (input->is_capturing()){
			float *in = (float*)data;

			RingBuffer &buf = input->buffer;
			AudioBuffer b;
			buf.write_ref(b, frames);
			b.deinterleave(in, input->num_channels);
			buf.write_ref_done(b);

			int done = b.length;
			if (done < frames){
				buf.write_ref(b, frames - done);
				b.deinterleave(&in[input->num_channels * done], input->num_channels);
				buf.write_ref_done(b);
			}
		}

		pa_stream_drop(p);
		input->_pulse_test_error("pa_stream_drop");
	}
	//msg_write(">");
}
void input_notify_callback(pa_stream *p, void *userdata)
{
	printf("sstate... %p:  ", p);
	int s = pa_stream_get_state(p);
	if (s == PA_STREAM_UNCONNECTED)
		printf("unconnected");
	if (s == PA_STREAM_READY)
		printf("ready");
	if (s == PA_STREAM_TERMINATED)
		printf("terminated");
	printf("\n");
}


void input_success_callback(pa_stream *s, int success, void *userdata)
{
	msg_write("--success");
}
#endif


#if HAS_LIB_PORTAUDIO

int InputStreamAudio::portaudio_stream_request_callback(const void *inputBuffer, void *outputBuffer,
                                                    unsigned long frames,
                                                    const PaStreamCallbackTimeInfo* timeInfo,
                                                    PaStreamCallbackFlags statusFlags,
                                                    void *userData)
{
	//printf("request %d\n", (int)frames);
	InputStreamAudio *input = (InputStreamAudio*)userData;

	(void)outputBuffer; /* Prevent unused variable warning. */
	float *in = (float*) inputBuffer;


	if (in){
		if (input->is_capturing()){

			RingBuffer &buf = input->buffer;
			AudioBuffer b;
			buf.write_ref(b, frames);
			b.deinterleave(in, input->num_channels);
			buf.write_ref_done(b);

			int done = b.length;
			if (done < (int)frames){
				buf.write_ref(b, frames - done);
				b.deinterleave(&in[input->num_channels * done], input->num_channels);
				buf.write_ref_done(b);
			}
		}
	}
	return 0;
}

#endif



void InputStreamAudio::SyncData::reset()
{
	num_points = 0;
	delay_sum = 0;
	samples_in = 0;
	/*if (tsunami->output->IsPlaying())
		offset_out = tsunami->output->GetRange().offset;*/ // TODO
}

void InputStreamAudio::SyncData::add(int samples)
{
#if 0
	if (tsunami->win->view->isPlaying()){
		samples_in += samples;
		/*delay_sum += (tsunami->output->GetPos() - offset_out - samples_in);*/ // TODO
		num_points ++;
	}
#endif
}

int InputStreamAudio::SyncData::get_delay()
{
	if (num_points > 0)
		return (delay_sum / num_points);
	return 0;
}

InputStreamAudio::Output::Output(InputStreamAudio *s) :
	Port(SignalType::AUDIO, "out")
{
	stream = s;
}

int InputStreamAudio::Output::read_audio(AudioBuffer &buf)
{
	//printf("read %d %d\n", buf.length, stream->buffer.available());
	if (stream->buffer.available() < buf.length)
		return NOT_ENOUGH_DATA;

	int r = stream->buffer.read(buf);

	return r;
}

extern bool ugly_hack_slow;

InputStreamAudio::InputStreamAudio(Session *_session) :
	Module(ModuleType::STREAM, "AudioInput"),
	buffer(1048576)
{
	set_session_etc(_session, "", nullptr);
	_sample_rate = session->sample_rate();
	chunk_size = DEFAULT_CHUNK_SIZE;
	num_channels = 0;

	state = State::NO_DEVICE;
#if HAS_LIB_PULSEAUDIO
	pulse_stream = nullptr;
#endif
#if HAS_LIB_PORTAUDIO
	portaudio_stream = nullptr;
#endif

	out = new Output(this);
	port_out.add(out);

	device = session->device_manager->choose_device(DeviceType::AUDIO_INPUT);
	dev_man = session->device_manager;
	playback_delay_const = 0;
	if (device){
		playback_delay_const = device->latency;
		num_channels = device->channels;
	}
}

InputStreamAudio::~InputStreamAudio()
{
	if (state == State::CAPTURING)
		_pause();
	if (state == State::PAUSED)
		_kill_dev();
}

void InputStreamAudio::__init__(Session *session)
{
	new(this) InputStreamAudio(session);
}

void InputStreamAudio::__delete__()
{
	this->InputStreamAudio::~InputStreamAudio();
}

void InputStreamAudio::set_chunk_size(int size)
{
	if (size > 0)
		chunk_size = size;
	else
		chunk_size = DEFAULT_CHUNK_SIZE;
}

Device *InputStreamAudio::get_device()
{
	return device;
}

void InputStreamAudio::set_device(Device *_device)
{
	auto old_state = state;
	if (state == State::CAPTURING)
		_pause();
	if (state == State::PAUSED)
		_kill_dev();

	device = _device;
	playback_delay_const = device->latency;

	if (old_state == State::CAPTURING)
		start();


	//tsunami->log->info(format("input device '%s' chosen", Pa_GetDeviceInfo(pa_device_no)->name));
}

void InputStreamAudio::_kill_dev()
{
	if (state != State::PAUSED)
		return;
	session->debug("input", "kill device");

#if HAS_LIB_PULSEAUDIO
	if (dev_man->audio_api == DeviceManager::ApiType::PULSE){
		pa_stream_disconnect(pulse_stream);
		_pulse_test_error("pa_stream_disconnect");

		for (int i=0; i<1000; i++){
			if (pa_stream_get_state(pulse_stream) == PA_STREAM_TERMINATED){
				break;
			}
			hui::Sleep(0.001f);
		}

		pa_stream_unref(pulse_stream);
		_pulse_test_error("pa_stream_unref");
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

void InputStreamAudio::stop()
{
	session->i(_("capture audio stop"));
	_pause();
}

void InputStreamAudio::_pause()
{
	if (state != State::CAPTURING)
		return;
	session->debug("input", "pause");


#if HAS_LIB_PULSEAUDIO
	if (pulse_stream){
		pa_operation *op = pa_stream_cork(pulse_stream, true, nullptr, nullptr);
		_pulse_test_error("pa_stream_cork");
		pa_wait_op(session, op);
	}
#endif
#if HAS_LIB_PORTAUDIO
	if (portaudio_stream){
		// often crashes here... might be a bug in the libraries...?!?
		PaError err = Pa_StopStream(portaudio_stream);
		_portaudio_test_error(err, "Pa_StopStream");
	}
#endif

	state = State::PAUSED;
	//buffer.clear();
}

void InputStreamAudio::_create_dev()
{
	if (state != State::NO_DEVICE)
		return;

	session->debug("input", "create device");

	num_channels = 2;

#if HAS_LIB_PULSEAUDIO
	if (dev_man->audio_api == DeviceManager::ApiType::PULSE){
		pa_sample_spec ss;
		ss.rate = _sample_rate;
		ss.channels = 2;
		ss.format = PA_SAMPLE_FLOAT32LE;
		pulse_stream = pa_stream_new(session->device_manager->pulse_context, "stream-in", &ss, nullptr);
		_pulse_test_error("pa_stream_new");


		pa_stream_set_read_callback(pulse_stream, &pulse_stream_request_callback, this);
		//pa_stream_set_state_callback(pulse_stream, &input_notify_callback, NULL);

		pa_buffer_attr attr_in;
	//	attr_in.fragsize = -1;
		attr_in.fragsize = chunk_size;
		attr_in.maxlength = -1;
		attr_in.minreq = -1;
		attr_in.tlength = -1;
		attr_in.prebuf = -1;
		const char *dev = nullptr;
		if (!device->is_default())
			dev = device->internal_name.c_str();
		pa_stream_connect_record(pulse_stream, dev, &attr_in, (pa_stream_flags_t)PA_STREAM_ADJUST_LATENCY);
		// without PA_STREAM_ADJUST_LATENCY, we will get big chunks (split into many small ones, but still "clustered")
		_pulse_test_error("pa_stream_connect_record");

		if (!pa_wait_stream_ready(pulse_stream)){

			session->e("pa_wait_for_stream_ready");
			return;
		}
	}
#endif

#if HAS_LIB_PORTAUDIO
	if (dev_man->audio_api == DeviceManager::ApiType::PORTAUDIO){
		session->i("open def stream");

		if (device->is_default()){
			PaError err = Pa_OpenDefaultStream(&portaudio_stream, 2, 0, paFloat32, _sample_rate, 256,
					&portaudio_stream_request_callback, this);
			_portaudio_test_error(err, "Pa_OpenDefaultStream");
		}else{
			PaStreamParameters params;
			num_channels = min(device->channels, 2);
			params.channelCount = num_channels;
			params.sampleFormat = paFloat32;
			params.device = device->index_in_lib;
			params.hostApiSpecificStreamInfo = nullptr;
			params.suggestedLatency = 0;
			PaError err = Pa_OpenStream(&portaudio_stream, &params, nullptr, _sample_rate, 256,
					paNoFlag, &portaudio_stream_request_callback, this);
			_portaudio_test_error(err, "Pa_OpenStream");
		}
	}
#endif

	state = State::PAUSED;
}

void InputStreamAudio::_unpause()
{
	if (state != State::PAUSED)
		return;
	session->debug("input", "unpause");

#if HAS_LIB_PULSEAUDIO
	if (dev_man->audio_api == DeviceManager::ApiType::PULSE){
		if (pulse_stream){
			pa_operation *op = pa_stream_cork(pulse_stream, false, nullptr, nullptr);
			_pulse_test_error("pa_stream_cork");
			pa_wait_op(session, op);
		}
	}
#endif
#if HAS_LIB_PORTAUDIO
	if (dev_man->audio_api == DeviceManager::ApiType::PORTAUDIO){
		if (portaudio_stream){
			PaError err = Pa_StartStream(portaudio_stream);
			_portaudio_test_error(err, "Pa_StartStream");
		}
	}
#endif

	state = State::CAPTURING;
}

bool InputStreamAudio::start()
{
	session->i(_("capture audio start"));
	if (state == State::NO_DEVICE)
		_create_dev();

	_unpause();
	return state == State::CAPTURING;
}

bool InputStreamAudio::_pulse_test_error(const string &msg)
{
#if HAS_LIB_PULSEAUDIO
	int e = pa_context_errno(session->device_manager->pulse_context);
	if (e != 0)
		session->e(msg + " (input): " + pa_strerror(e));
	return (e != 0);
#endif
	return false;
}

#if HAS_LIB_PORTAUDIO
bool InputStreamAudio::_portaudio_test_error(PaError err, const string &msg)
{
	if (err != paNoError){
		session->e(msg + ": (input): " + Pa_GetErrorText(err));
		return true;
	}
	return false;
}
#endif

float InputStreamAudio::get_playback_delay_const()
{
	return playback_delay_const;
}

void InputStreamAudio::set_playback_delay_const(float f)
{
	playback_delay_const = f;
	hui::Config.set_float("Input.PlaybackDelay", playback_delay_const);
}

void InputStreamAudio::reset_sync()
{
	sync.reset();
}

bool InputStreamAudio::is_capturing()
{
	return state == State::CAPTURING;
}

int InputStreamAudio::get_delay()
{
	return sync.get_delay() - playback_delay_const * (float)_sample_rate / 1000.0f;
}

void InputStreamAudio::command(ModuleCommand cmd)
{
	if (cmd == ModuleCommand::START){
		start();
	}else if (cmd == ModuleCommand::STOP){
		stop();
	}else if (cmd == ModuleCommand::PREPARE_START){
		if (state == State::NO_DEVICE)
			_create_dev();
	}
}
