/*
 * AudioInputAudio.cpp
 *
 *  Created on: 26.03.2012
 *      Author: michi
 */

#include "AudioInputAudio.h"
#include "AudioStream.h"
#include "AudioOutput.h"
#include "../lib/hui/hui.h"
#include "../Tsunami.h"
#include "../TsunamiWindow.h"
#include "../Stuff/Log.h"
#include "../View/AudioView.h"

#include <pulse/pulseaudio.h>


string AudioInputAudio::temp_filename;
string AudioInputAudio::favorite_device;
string AudioInputAudio::cur_temp_filename;
float AudioInputAudio::playback_delay_const;


static const int DEFAULT_CHUNK_SIZE = 512;
static const float DEFAULT_UPDATE_TIME = 0.005f;
const string AudioInputAudio::MESSAGE_CAPTURE = "Capture";


extern void pa_wait_op(pa_operation *op); // -> AudioOutput.cpp
extern bool pa_wait_stream_ready(pa_stream *s); // -> AudioStream.cpp


void pa_source_info_callback(pa_context *c, const pa_source_info *i, int eol, void *userdata)
{
	if ((eol > 0) or (!i))
		return;

	Array<string> *devices = (Array<string>*)userdata;
	if (devices)
		devices->add(i->name);
}



void AudioInputAudio::input_request_callback(pa_stream *p, size_t nbytes, void *userdata)
{
	//printf("read %d\n", (int)nbytes);
	AudioInputAudio *input = (AudioInputAudio*)userdata;
	if (!input->isCapturing())
		return;

	const void *data;
	pa_stream_peek(p, &data, &nbytes);
	input->testError("pa_stream_peek");
	int frames = nbytes / 4 / input->num_channels;

	if (data){
		float *in = (float*)data;

		RingBuffer &buf = input->current_buffer;
		BufferBox b;
		buf.writeRef(b, frames);
		b.deinterleave(in, input->num_channels);

		int done = b.num;
		if (done < frames){
			buf.writeRef(b, frames - done);
			b.deinterleave(&in[2 * done], input->num_channels);
		}

		pa_stream_drop(p);
		input->testError("pa_stream_drop");
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

void AudioInputAudio::SyncData::reset()
{
	num_points = 0;
	delay_sum = 0;
	samples_in = 0;
	/*if (tsunami->output->IsPlaying())
		offset_out = tsunami->output->GetRange().offset;*/ // TODO
}

void AudioInputAudio::SyncData::add(int samples)
{
	if (tsunami->win->view->stream->isPlaying()){
		samples_in += samples;
		/*delay_sum += (tsunami->output->GetPos() - offset_out - samples_in);*/ // TODO
		num_points ++;
	}
}

int AudioInputAudio::SyncData::getDelay()
{
	if (num_points > 0)
		return (delay_sum / num_points);
	return 0;
}


AudioInputAudio::AudioInputAudio(int _sample_rate) :
	PeakMeterSource("AudioInputAudio"),
	current_buffer(1048576)
{
	sample_rate = _sample_rate;
	capturing = false;
	_stream = NULL;

	favorite_device = HuiConfig.getStr("Input.ChosenDevice", "");
	chosen_device = favorite_device;
	playback_delay_const = HuiConfig.getFloat("Input.PlaybackDelay", 0.0f);
	temp_filename = HuiConfig.getStr("Input.TempFilename", "");
	temp_file = NULL;
	save_mode = false;

	running = false;
	update_dt = DEFAULT_UPDATE_TIME;
	chunk_size = DEFAULT_CHUNK_SIZE;

	if (file_test_existence(getTempFilename()))
		tsunami->log->warn(_("alte Aufnahmedaten gefunden: ") + getTempFilename());
}

AudioInputAudio::~AudioInputAudio()
{
	stop();
}

void AudioInputAudio::__init__(int _sample_rate)
{
	new(this) AudioInputAudio(_sample_rate);
}

void AudioInputAudio::__delete__()
{
	stop();
	current_buffer.clear();
	buffer.clear();
	chosen_device.clear();
}


Array<string> AudioInputAudio::getDevices()
{
	Array<string> devices;

	pa_operation *op = pa_context_get_source_info_list(tsunami->output->context, pa_source_info_callback, &devices);
	if (!testError("pa_context_get_source_info_list"))
		pa_wait_op(op);

	return devices;
}

string AudioInputAudio::getFavoriteDevice()
{
	return favorite_device;
}

string AudioInputAudio::getChosenDevice()
{
	/*if (pa_device_no >= 0)
		if (pa_device_no == Pa_GetDefaultInputDevice())
			return "";*/
	return chosen_device;
}

void AudioInputAudio::setFavoriteDevice(const string &device)
{
	favorite_device = device;
}

void AudioInputAudio::setDevice(const string &device)
{
	HuiConfig.setStr("Input.ChosenDevice", device);


	Array<string> devs = getDevices();

	// valid?
	bool valid = (device == "");
	foreach(string &d, devs)
		if (d == device)
			valid = true;


	if (valid){
		chosen_device = device;
	}else{
		tsunami->log->error(format("input device '%s' not found. Using default.", device.c_str()));
		chosen_device = "";
	}
	setFavoriteDevice(chosen_device);

	if (capturing){
		stop();
		start();
	}

	//tsunami->log->info(format("input device '%s' chosen", Pa_GetDeviceInfo(pa_device_no)->name));
}

void AudioInputAudio::stop()
{
	msg_db_f("AudioInputAudio.stop", 1);
	if (!capturing)
		return;
	_stopUpdate();

	pa_stream_disconnect(_stream);
	testError("disconnect");

	for (int i=0; i<1000; i++){
		if (pa_stream_get_state(_stream) == PA_STREAM_TERMINATED)
			break;
		HuiSleep(0.001f);
	}

	pa_stream_unref(_stream);
	testError("unref");

	capturing = false;
	accumulating = false;
	current_buffer.clear();
	if (temp_file){
		delete(temp_file);
		temp_file = NULL;
		file_delete(cur_temp_filename);
	}
}

bool AudioInputAudio::start()
{
	msg_db_f("AudioInputAudio.start", 1);
	if (capturing)
		stop();

	setDevice(chosen_device);

	accumulating = false;
	num_channels = 2;

	pa_sample_spec ss;
	ss.rate = sample_rate;
	ss.channels = 2;
	ss.format = PA_SAMPLE_FLOAT32LE;
	_stream = pa_stream_new(tsunami->output->context, "stream-in", &ss, NULL);
	testError("pa_stream_new");


	pa_stream_set_read_callback(_stream, &input_request_callback, this);
	//pa_stream_set_state_callback(_stream, &input_notify_callback, NULL);

	pa_buffer_attr attr_in;
//	attr_in.fragsize = -1;
	attr_in.fragsize = chunk_size;
	attr_in.maxlength = -1;
	attr_in.minreq = -1;
	attr_in.tlength = -1;
	attr_in.prebuf = -1;
	const char *dev = NULL;
	if (chosen_device != "")
		dev = chosen_device.c_str();
	pa_stream_connect_record(_stream, dev, &attr_in, (pa_stream_flags_t)PA_STREAM_ADJUST_LATENCY);
	// without PA_STREAM_ADJUST_LATENCY, we will get big chunks (split into many small ones, but still "clustered")
	testError("pa_stream_connect_record");

	if (!pa_wait_stream_ready(_stream)){
		tsunami->log->error("pa_wait_for_stream_ready");
		return false;
	}

	capturing = true;

	if (save_mode){
		cur_temp_filename = getTempFilename();
		temp_file = FileCreate(getTempFilename());
		temp_file->SetBinaryMode(true);
	}

	_startUpdate();

	resetSync();
	return capturing;
}

bool AudioInputAudio::testError(const string &msg)
{
	int e = pa_context_errno(tsunami->output->context);
	if (e != 0)
		tsunami->log->error(msg + " (input): " + pa_strerror(e));
	return (e != 0);
}

float AudioInputAudio::getPlaybackDelayConst()
{
	return playback_delay_const;
}

void AudioInputAudio::accumulate(bool enable)
{
	//resetAccumulation();
	current_buffer.clear();
	accumulating = enable;
}

void AudioInputAudio::resetAccumulation()
{
	buffer.clear();
}

int AudioInputAudio::getSampleCount()
{
	return buffer.num;
}

void AudioInputAudio::setPlaybackDelayConst(float f)
{
	playback_delay_const = f;
	HuiConfig.setFloat("Input.PlaybackDelay", playback_delay_const);
}

void AudioInputAudio::setChunkSize(int size)
{
	if (size > 0)
		chunk_size = size;
	else
		chunk_size = DEFAULT_CHUNK_SIZE;
}

void AudioInputAudio::setUpdateDt(float dt)
{
	if (dt > 0)
		update_dt = dt;
	else
		update_dt = DEFAULT_UPDATE_TIME;
}

int AudioInputAudio::doCapturing()
{
	if (!capturing)
		return 0;

	msg_db_f("DoCapturing", 1);

	int avail = current_buffer.available();
	sync.add(avail);

	if (!accumulating)
		return avail;

	BufferBox b;
	current_buffer.readRef(b, avail);
	buffer.append(b);

	if (temp_file){
		// write to file
		string data;
		b.exports(data, 2, SAMPLE_FORMAT_32_FLOAT);

		temp_file->WriteBuffer(&data[0], data.num);
	}

	return avail;
}

void AudioInputAudio::resetSync()
{
	sync.reset();
}

void AudioInputAudio::getSomeSamples(BufferBox &buf, int num_samples)
{
	current_buffer.peekRef(buf, -num_samples);
}

float AudioInputAudio::getSampleRate()
{
	return sample_rate;
}

bool AudioInputAudio::isCapturing()
{
	return capturing;
}

int AudioInputAudio::getState()
{
	if (isCapturing())
		return STATE_PLAYING;
	return STATE_STOPPED;
}

int AudioInputAudio::getDelay()
{
	return sync.getDelay() - playback_delay_const * (float)sample_rate / 1000.0f;
}

string AudioInputAudio::getDefaultTempFilename()
{
#ifdef OS_WINDOWS
	return "c:\\tsunami-input.raw";
#else
	return "/tmp/tsunami-input.raw";
#endif
}

string AudioInputAudio::getTempFilename()
{
	if (temp_filename.num > 0)
		return temp_filename;
	return getDefaultTempFilename();
}

void AudioInputAudio::setTempFilename(const string &filename)
{
	temp_filename = filename;
	HuiConfig.setStr("Input.TempFilename", temp_filename);
}

void AudioInputAudio::setSaveMode(bool enabled)
{
	save_mode = enabled;
}

void AudioInputAudio::_startUpdate()
{
	if (running)
		return;
	hui_runner_id = HuiRunRepeatedM(update_dt, this, &AudioInputAudio::update);
	running = true;
}

void AudioInputAudio::_stopUpdate()
{
	if (!running)
		return;
	HuiCancelRunner(hui_runner_id);
	hui_runner_id = -1;
	running = false;
}

void AudioInputAudio::update()
{
	if (doCapturing() > 0)
		notify(MESSAGE_CAPTURE);

	running = isCapturing();
}
