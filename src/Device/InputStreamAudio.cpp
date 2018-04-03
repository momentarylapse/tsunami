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

#if HAS_LIB_PULSEAUDIO
#include <pulse/pulseaudio.h>
#endif

#if HAS_LIB_PORTAUDIO
#include <portaudio.h>
#endif


float InputStreamAudio::playback_delay_const;


const string InputStreamAudio::MESSAGE_CAPTURE = "Capture";
static const int DEFAULT_CHUNK_SIZE = 512;
static const float DEFAULT_UPDATE_TIME = 0.005f;


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
			buf.writeRef(b, frames);
			b.deinterleave(in, input->num_channels);

			int done = b.length;
			if (done < frames){
				buf.writeRef(b, frames - done);
				b.deinterleave(&in[input->num_channels * done], input->num_channels);
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

InputStreamAudio::Output::Output(InputStreamAudio *s)
{
	stream = s;
}

int InputStreamAudio::Output::read(AudioBuffer &buf)
{
	//printf("read %d %d\n", buf.length, stream->buffer.available());
	if (stream->buffer.available() < buf.length)
		return NOT_ENOUGH_DATA;

	int r = stream->buffer.read(buf);

	if (stream->backup_file){
		// write to file
		string data;
		buf.exports(data, 2, SAMPLE_FORMAT_32_FLOAT);
		stream->backup_file->write_buffer(&data[0], data.num);
	}

	return r;
}


InputStreamAudio::InputStreamAudio(Session *_session) :
	buffer(1048576)
{
//	printf("input new\n");
	session = _session;
	_sample_rate = session->sample_rate();
	chunk_size = -1;
	update_dt = -1;
	update_dt = DEFAULT_UPDATE_TIME;
	chunk_size = DEFAULT_CHUNK_SIZE;
	num_channels = 0;

	capturing = false;
#if HAS_LIB_PULSEAUDIO
	pulse_stream = NULL;
#endif
#if HAS_LIB_PORTAUDIO
	portaudio_stream = NULL;
#endif

	out = new Output(this);

	device = session->device_manager->chooseDevice(Device::Type::AUDIO_INPUT);
	api = session->device_manager->audio_api;
	playback_delay_const = 0;
	if (device){
		playback_delay_const = device->latency;
		num_channels = device->channels;
	}
	backup_file = NULL;
	backup_mode = BACKUP_MODE_NONE;

	running = false;
	hui_runner_id = -1;
}

InputStreamAudio::~InputStreamAudio()
{
//	printf("input del\n");
	stop();
	delete out;
}

void InputStreamAudio::__init__(Session *session)
{
	new(this) InputStreamAudio(session);
}

void InputStreamAudio::__delete__()
{
	this->InputStreamAudio::~InputStreamAudio();
}

void InputStreamAudio::set_backup_mode(int mode)
{
	backup_mode = mode;
}

void InputStreamAudio::set_chunk_size(int size)
{
	if (size > 0)
		chunk_size = size;
	else
		chunk_size = DEFAULT_CHUNK_SIZE;
}

void InputStreamAudio::set_update_dt(float dt)
{
	if (dt > 0)
		update_dt = dt;
	else
		update_dt = DEFAULT_UPDATE_TIME;
}

Device *InputStreamAudio::get_device()
{
	return device;
}

void InputStreamAudio::set_device(Device *_device)
{
	device = _device;
	playback_delay_const = device->latency;

	if (capturing){
		stop();
		start();
	}

	//tsunami->log->info(format("input device '%s' chosen", Pa_GetDeviceInfo(pa_device_no)->name));
}

void InputStreamAudio::stop()
{
//	printf("input stop\n");
	_stop();
}

void InputStreamAudio::_stop()
{
	if (!capturing)
		return;
	session->i(_("capture audio stop"));
	_stop_update();

#if HAS_LIB_PULSEAUDIO
	if (api == DeviceManager::API_PULSE){
//	printf("disconnect\n");
	pa_stream_disconnect(pulse_stream);
	_pulse_test_error("disconnect");

	for (int i=0; i<1000; i++){
		if (pa_stream_get_state(pulse_stream) == PA_STREAM_TERMINATED){
//			printf("terminated\n");
			break;
		}
		hui::Sleep(0.001f);
	}
//	printf("unref\n");

	pa_stream_unref(pulse_stream);
	_pulse_test_error("unref");
	}
#endif

	capturing = false;
	buffer.clear();
	if (backup_file){
		BackupManager::done(backup_file);
		backup_file = NULL;
		//if (backup_mode != BACKUP_MODE_KEEP)
		//	file_delete(cur_backup_filename);
	}
}

bool InputStreamAudio::start()
{
//	printf("input start\n");
	if (capturing)
		_stop();

	session->i(_("capture audio start"));

	num_channels = 2;

#if HAS_LIB_PULSEAUDIO
	if (api == DeviceManager::API_PULSE){
	pa_sample_spec ss;
	ss.rate = _sample_rate;
	ss.channels = 2;
	ss.format = PA_SAMPLE_FLOAT32LE;
	pulse_stream = pa_stream_new(session->device_manager->pulse_context, "stream-in", &ss, NULL);
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
	const char *dev = NULL;
	if (!device->is_default())
		dev = device->internal_name.c_str();
	pa_stream_connect_record(pulse_stream, dev, &attr_in, (pa_stream_flags_t)PA_STREAM_ADJUST_LATENCY);
	// without PA_STREAM_ADJUST_LATENCY, we will get big chunks (split into many small ones, but still "clustered")
	_pulse_test_error("pa_stream_connect_record");

	if (!pa_wait_stream_ready(pulse_stream)){

		session->e("pa_wait_for_stream_ready");
		return false;
	}
	}
#endif

	capturing = true;

	if (backup_mode != BACKUP_MODE_NONE)
		backup_file = BackupManager::create_file("raw", session);

	_start_update();

	reset_sync();
	return capturing;
}

bool InputStreamAudio::_pulse_test_error(const string &msg)
{
#if HAS_LIB_PULSEAUDIO
	int e = pa_context_errno(session->device_manager->pulse_context);
	if (e != 0)
		{}//hui::RunLater(0.001f, std::bind(&Session::e, session, msg + " (input): " + pa_strerror(e)));
	// make sure errors are handled in the gui thread...
	//tsunami->log->error(msg + " (input): " + pa_strerror(e));
	return (e != 0);
#endif
	return false;
}

float InputStreamAudio::get_playback_delay_const()
{
	return playback_delay_const;
}

void InputStreamAudio::set_playback_delay_const(float f)
{
	playback_delay_const = f;
	hui::Config.setFloat("Input.PlaybackDelay", playback_delay_const);
}

int InputStreamAudio::do_capturing()
{
	if (!capturing)
		return 0;

	int avail = buffer.available();
	sync.add(avail);

	return avail;
}

void InputStreamAudio::reset_sync()
{
	sync.reset();
}

bool InputStreamAudio::is_capturing()
{
	return capturing;
}

int InputStreamAudio::get_delay()
{
	return sync.get_delay() - playback_delay_const * (float)_sample_rate / 1000.0f;
}

void InputStreamAudio::_start_update()
{
	if (running)
		return;
	hui_runner_id = hui::RunRepeated(update_dt, std::bind(&InputStreamAudio::update, this));
	running = true;
}

void InputStreamAudio::_stop_update()
{
	if (!running)
		return;
	hui::CancelRunner(hui_runner_id);
	hui_runner_id = -1;
	running = false;
}

void InputStreamAudio::update()
{
	if (do_capturing() > 0)
		notify(MESSAGE_CAPTURE);

	running = is_capturing();
}
