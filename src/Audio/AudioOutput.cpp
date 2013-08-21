/*
 * AudioOutput.cpp
 *
 *  Created on: 26.03.2012
 *      Author: michi
 */

#include "AudioOutput.h"
#include "../Tsunami.h"
#include "AudioRenderer.h"
#include "../Stuff/Log.h"


#ifdef NIX_OS_WINDOWS
	#include <al.h>
	#include <alut.h>
	#include <alc.h>
	#pragma comment(lib,"alut.lib")
	#pragma comment(lib,"OpenAL32.lib")
	/*#pragma comment(lib,"libogg.lib")
	#pragma comment(lib,"libvorbis.lib")
	#pragma comment(lib,"libvorbisfile.lib")*/

#else
	#include <AL/al.h>
	#include <AL/alut.h>
	#include <AL/alc.h>
#endif

//#define DEFAULT_BUFFER_SIZE		131072
#define DEFAULT_BUFFER_SIZE		32768
//#define DEFAULT_BUFFER_SIZE		16384

#define UPDATE_TIME		0.050f

// sends notifications:
//   "StateChange", "Update"

AudioOutput::AudioOutput() :
	PeakMeterSource("AudioOutput")
{
	al_initialized = false;
	al_last_error = AL_NO_ERROR;

	renderer = NULL;
	generate_func = NULL;

	playing = false;
	volume = 1;

	al_context = NULL;
	al_dev = NULL;
	buffer[0] = -1;
	buffer[1] = -1;
	source = -1;
	data_samples = 0;

	ChosenDevice = HuiConfigReadStr("Output.ChosenDevice", "");
	volume = HuiConfigReadFloat("Output.Volume", 1.0f);

	Init();
}

AudioOutput::~AudioOutput()
{
	HuiConfigWriteStr("Output.ChosenDevice", ChosenDevice);
	HuiConfigWriteFloat("Output.Volume", volume);
}

void AudioOutput::SetDevice(const string &device)
{
	ChosenDevice = device;
	HuiConfigWriteStr("ChosenOutputDevice", ChosenDevice);
	HuiSaveConfigFile();
	tsunami->log->Warning(_("Das neue Ger&at wird erst beim n&achsten Start verwendet!"));
	//KillPreview();
	//PreviewInit();
}

void AudioOutput::Init()
{
	if (al_initialized)
		return;
	msg_db_f("Output.Init", 1);

	// which device to use?
	string dev_name;
	const char *s = alcGetString(NULL, ALC_DEVICE_SPECIFIER);
	Device.clear();
	while(*s != 0){
		Device.add(string(s));
		if (string(s) == ChosenDevice)
			dev_name = s;
		s += strlen(s) + 1;
	}
	if (dev_name.num == 0)
		ChosenDevice = "";

	// try to open manually
	bool ok = false;
	if (dev_name.num > 0){
		al_dev = alcOpenDevice(dev_name.c_str());
		if (al_dev){
			TestError("alcOpenDevice (init)");
			al_context = alcCreateContext(al_dev, NULL);
			TestError("alcCreateContext (init)");
			if (al_context){
				if (alcMakeContextCurrent(al_context)){
					tsunami->log->Info(_("benutze OpenAl Device: ") + dev_name);
					ok = true;
				}
				TestError("alcMakeContextCurrent (init)");
			}
		}
	}

	// failed -> use automatic method
	if (!ok){
		ok = alutInit(NULL, 0);
		TestError("alutInit (init)");
		al_dev = NULL;
		al_context = NULL;
	}

	if (!ok){
		tsunami->log->Error(string("OpenAL init: ") + alutGetErrorString(al_last_error));
		return;
	}

	//SetListenerValues();
	TestError("init...");

	//alGenBuffers(1, &buffer);
	alGenSources(1, &source);
	al_initialized = true;
}

void AudioOutput::Kill()
{
	if (!al_initialized)
		return;
	msg_db_f("Output.Kill",1);
	Stop();
	TestError("? (pre kill)");
	if (buffer[0] >= 0)
		alDeleteBuffers(2, (ALuint*)buffer);
	TestError("alDeteleBuffers (kill)");
	if (source >= 0)
		alDeleteSources(1, &source);
	TestError("alDeleteSources (kill)");
	// close devices
	if (al_dev){
		// manually
		//msg_write("current context...");
		alcMakeContextCurrent(NULL);
		TestError("alcMakeContextCurrent (kill)");
		//msg_write("destroy context...");
		alcDestroyContext(al_context);
		TestError("alcDestroyContext (kill)");
		//msg_write("close device...");
		if (!alcCloseDevice(al_dev))
			TestError("alcCloseDevice (kill)");
		//msg_write("ok");
	}else{
		// automatically
		int i = alutExit();
		if (i == 0)
			msg_error((char*)alutGetErrorString(alutGetError()));
	}
	al_initialized = false;
}


string ALError(int err)
{
	if (err==AL_NO_ERROR)
		return "AL_NO_ERROR";
	if (err==AL_INVALID_NAME)
		return "AL_INVALID_NAME";
	if (err==AL_INVALID_ENUM)
		return "AL_INVALID_ENUM";
	if (err==AL_INVALID_VALUE)
		return "AL_INVALID_VALUE";
	if (err==AL_INVALID_OPERATION)
		return "AL_INVALID_OPERATION";
	if (err==AL_OUT_OF_MEMORY)
		return "AL_OUT_OF_MEMORY";
	return i2s(err);
}

void AudioOutput::stop_play()
{
	if (!playing)
		return;

	TestError("?  (prestop)");
	alSourceStop(source);
	TestError("alSourceStop (stop)");
	int queued;
	alGetSourcei(source, AL_BUFFERS_QUEUED, &queued);
	TestError("alGetSourcei(queued) (stop)");
	while(queued--){
		ALuint buf;
		alSourceUnqueueBuffers(source, 1, &buf);
		TestError(format("alSourceUnqueueBuffers(%d) (stop)", queued));
	}

	if (renderer)
		renderer->CleanUp();
}

void AudioOutput::Stop()
{
	if (!playing)
		return;
	msg_db_f("Output.Stop", 1);

	stop_play();

	alDeleteBuffers(2, (ALuint*)buffer);
	TestError("alDeleteBuffers (stop)");
	buffer[0] = -1;
	playing = false;
	renderer = NULL;

	Notify("StateChange");
}

void AudioOutput::Pause()
{
	if (!playing)
		return;
	int param;
	alGetSourcei(source, AL_SOURCE_STATE, &param);
	if (param == AL_PLAYING)
		alSourcePause(source);
	else if (param == AL_PAUSED)
		alSourcePlay(source);
	Notify("StateChange");
}

bool AudioOutput::stream(int buf)
{
	msg_db_f("stream", 1);
	BufferBox *b = (buf == buffer[0]) ? &box[0] : &box[1];
	int size = 0;
	b->resize(buffer_size);
	b->offset = stream_offset_next;
	if (renderer){
		size = renderer->read(*b);
		//msg_write(size);
	}else if (generate_func){
		size = (*generate_func)(*b);
	}
	if (size == 0)
		return false;
	b->offset = stream_offset_next;
	b->get_16bit_buffer(data);
	alBufferData(buf, AL_FORMAT_STEREO16, &data[0], size * 4, sample_rate);
	TestError("alBufferData (stream)");

	stream_offset_next += size;
	return true;
}

void AudioOutput::start_play(int pos)
{

	stream_offset_next = pos;
	buffer_size = DEFAULT_BUFFER_SIZE;

	int num_buffers = 0;
	if (stream(buffer[0]))
		num_buffers ++;
	if (stream(buffer[1]))
		num_buffers ++;

	alSourcef (source, AL_PITCH,    1.0f);
	alSourcef (source, AL_GAIN,     volume);
//	alSourcefv(source, AL_POSITION, SourcePos);
//	alSourcefv(source, AL_VELOCITY, SourceVel);
	alSourcei (source, AL_LOOPING,  false);
	if (TestError("alSourcef... (play)"))
		return;

	cur_buffer_no = 0;
	alSourceQueueBuffers(source, num_buffers, (ALuint*)buffer);
	TestError("alSourceQueueBuffers (play)");

	alSourcePlay(source);
	if (TestError("alSourcePlay (play)"))
		return;
}

void AudioOutput::Play(AudioRenderer *r)
{
	msg_db_f("PreviewPlay", 1);

	if (!al_initialized)
		Init();

	if (playing)
		Stop();

	alGenBuffers(2, (ALuint*)buffer);
	TestError("alGenBuffers (play)");

	renderer = r;
	generate_func = NULL;
	sample_rate = tsunami->audio->sample_rate; //r->sample_rate; // FIXME

	start_play(0);

	playing = true;

	HuiRunLaterM(UPDATE_TIME, this, &AudioOutput::Update);

	Notify("StateChange");
}

void AudioOutput::PlayGenerated(void *func, int _sample_rate)
{
	msg_db_f("PlayGenerated", 1);

	if (!al_initialized)
		Init();

	if (playing)
		Stop();

	alGenBuffers(2, (ALuint*)buffer);
	TestError("alGenBuffers (play)");

	renderer = NULL;
	generate_func = (generate_func_t*)func;
	sample_rate = _sample_rate;

	start_play(0);

	playing = true;

	HuiRunLaterM(UPDATE_TIME, this, &AudioOutput::Update);

	Notify("StateChange");
}

bool AudioOutput::IsPlaying()
{
	return playing;
}

int AudioOutput::GetPos()
{
	if (playing){
		int param = 0;
		alGetSourcei(source, AL_SOURCE_STATE, &param);
		TestError("alGetSourcei1 (getpos)");
		if ((param == AL_PLAYING) || (param == AL_PAUSED)){
			alGetSourcei(source, AL_SAMPLE_OFFSET, &param);
			TestError("alGetSourcei2 (getpos)");
			return box[cur_buffer_no].offset + param;
		}
	}
	return 0;
}

float AudioOutput::GetVolume()
{
	return volume;
}

void AudioOutput::SetVolume(float _volume)
{
	volume = _volume;
}

float AudioOutput::GetSampleRate()
{
	return sample_rate;
}

void AudioOutput::GetSomeSamples(BufferBox &buf, int num_samples)
{
	if (!playing)
		return;

	// (sample) position within current stream/buffer
	int dpos = 0;
	alGetSourcei(source, AL_SAMPLE_OFFSET, &dpos);

	buf.set_as_ref(box[cur_buffer_no], dpos, min(num_samples, box[cur_buffer_no].num - dpos));
}

bool AudioOutput::TestError(const string &msg)
{
	int error;
	if (al_dev)
		error = alcGetError(al_dev);
	else
		error = alGetError();
	if (error != AL_NO_ERROR){
		al_last_error = error;
		//tsunami->log->Error();
		msg_error("OpenAL operation: " + msg);
		msg_write(ALError(error));
		//msg_write(alutGetErrorString(error));
		return true;
	}
	return false;
}

void AudioOutput::Update()
{
	msg_db_f("Out.Update", 1);
	TestError("idle");
	if (playing){
		alSourcef(source, AL_GAIN, volume);
		TestError("alGetSourcef(volume) (idle)");
		//msg_write("..");
		int processed;
		alGetSourcei(source, AL_BUFFERS_PROCESSED, &processed);
		TestError("alGetSourcei(processed) (idle)");
		while(processed--){
			cur_buffer_no = 1 - cur_buffer_no;
			ALuint buf;
			alSourceUnqueueBuffers(source, 1, &buf);
			TestError("alSourceUnqueueBuffers (idle)");
			if (stream(buf)){
				alSourceQueueBuffers(source, 1, &buf);
				TestError("alSourceQueueBuffers (idle)");
			}
		}
		Notify("Update");


		int param=0;
		alGetSourcei(source,AL_SOURCE_STATE, &param);
		TestError("alGetSourcei(state) (idle)");
		if ((param != AL_PLAYING) && (param != AL_PAUSED)){
			//msg_write("hat gestoppt...");
				Stop();
		}else{
		}

		if (buffer_size > DEFAULT_BUFFER_SIZE / 3)
			HuiRunLaterM(UPDATE_TIME, this, &AudioOutput::Update);
		else
			HuiRunLaterM(0, this, &AudioOutput::Update);
	}
}

