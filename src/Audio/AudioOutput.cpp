/*
 * AudioOutput.cpp
 *
 *  Created on: 26.03.2012
 *      Author: michi
 */

#include "AudioOutput.h"
#include "../Tsunami.h"


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

//#define AL_BUFFER_SIZE		131072
#define AL_BUFFER_SIZE		65536
//#define AL_BUFFER_SIZE		16384

#define UPDATE_TIME		30

AudioOutput::AudioOutput() :
	Observable("AudioOutput")
{
	al_initialized = false;
	al_last_error = AL_NO_ERROR;


	start = pos = 0;
	audio = NULL;
	stream_pos = 0;
	stream_size = 0;
	stream_pos_0 = 0;

	playing = false;
	loop = false;
	volume = 1;

	al_context = NULL;
	al_dev = NULL;
	buffer[0] = -1;
	buffer[1] = -1;
	source = -1;
	data_samples = 0;

	PlayLoop = false;

	ChosenDevice = HuiConfigReadStr("Output.ChosenDevice", "");
	volume = HuiConfigReadFloat("Output.Volume", 1.0f);

	Init();
}

AudioOutput::~AudioOutput()
{
	HuiConfigWriteStr("Output.ChosenDevice", ChosenDevice);
	HuiConfigWriteFloat("Output.Volume", volume);
}


void AudioOutput::Init()
{
	if (al_initialized)
		return;

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
			TestError2("alcOpenDevice (init)", (ALCdevice*)al_dev);
			al_context = alcCreateContext((ALCdevice*)al_dev, NULL);
			TestError2("alcCreateContext (init)", (ALCdevice*)al_dev);
			if (al_context){
				if (alcMakeContextCurrent((ALCcontext*)al_context)){
					tsunami->log->Info(_("benutze OpenAl Device: ") + dev_name);
					ok = true;
				}
				TestError2("alcMakeContextCurrent (init)", (ALCdevice*)al_dev);
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
		tsunami->log->Error(string("OpenAL init:\n") + alutGetErrorString(al_last_error));
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
	msg_db_r("KillPreview",1);
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
		TestError2("alcMakeContextCurrent (kill)", al_dev);
		//msg_write("destroy context...");
		alcDestroyContext((ALCcontext*)al_context);
		TestError2("alcDestroyContext (kill)", al_dev);
		//msg_write("close device...");
		if (!alcCloseDevice((ALCdevice*)al_dev))
			TestError2("alcCloseDevice (kill)", al_dev);
		//msg_write("ok");
	}else{
		// automatically
		int i = alutExit();
		if (i == 0)
			msg_error((char*)alutGetErrorString(alutGetError()));
	}
	al_initialized = false;
	msg_db_l(1);
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

void AudioOutput::Stop()
{
	if (!playing)
		return;
	msg_db_r("PreviewStop", 1);
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
	alDeleteBuffers(2, (ALuint*)buffer);
	TestError("alDeleteBuffers (stop)");
	buffer[0] = -1;
	playing = false;
	loop = false;

	Notify("Stop");
	msg_db_l(1);
}

bool AudioOutput::stream(int buf)
{
	msg_db_r("stream", 1);
	int size = min(AL_BUFFER_SIZE, stream_size - stream_pos);
	//msg_write(size);
	if (size == 0){
		msg_db_l(1);
		return false;
	}
	alBufferData(buf, AL_FORMAT_STEREO16, &data[stream_pos * 2], size * 4, audio->sample_rate);
	TestError("alBufferData (stream)");
	stream_pos += size;
	msg_db_l(1);
	return true;
}

void AudioOutput::Play(AudioFile *a, bool _loop)
{
	/*if ((source < 0) || (buffer < 0))
		return;*/
	msg_db_r("PreviewPlay", 1);

	if (!al_initialized)
		Init();

	Range _range = a->GetRange();
	if (!a->selection.empty())
		_range = a->selection;

	//AudioFileToBuffer(a, true, true);
	BufferBox buf = tsunami->renderer->RenderAudioFile(a, _range);
	buf.get_16bit_buffer(data);
	//int size = 4 * length;


	/*msg_write((int)buffer);
	msg_write((int)PVData);
	msg_write(size);
	msg_write(a->sample_rate);*/

	//
	if (playing)
		Stop();

	alGenBuffers(2, (ALuint*)buffer);
	TestError("alGenBuffers (play)");

	stream_pos = 0;
	stream_size = _range.length();
	stream_pos_0 = 0;
	audio = a;

	int num_buffers = 0;
	if (stream(buffer[0]))
		num_buffers ++;
	if (stream(buffer[1]))
		num_buffers ++;


	/*alGetError();
	alBufferData(buffer, AL_FORMAT_STEREO16, data, size, 44100);
	if (TestError("alBufferData (play)")){
		msg_db_l(1);
		return;
	}*/

//	alSourcei (source, AL_BUFFER,   buffer);
	alSourcef (source, AL_PITCH,    1.0f);
	alSourcef (source, AL_GAIN,     volume);
//	alSourcefv(source, AL_POSITION, SourcePos);
//	alSourcefv(source, AL_VELOCITY, SourceVel);
	alSourcei (source, AL_LOOPING,  false);
	if (TestError("alSourcef... (play)")){
		msg_db_l(1);
		return;
	}

	alSourceQueueBuffers(source, num_buffers, (ALuint*)buffer);
	TestError("alSourceQueueBuffers (play)");

	alSourcePlay(source);
	if (TestError("alSourcePlay (play)")){
		msg_db_l(1);
		return;
	}

	playing = true;
	start = _range.start();
	pos = start;
	loop = _loop;

	HuiRunLaterM(UPDATE_TIME, this, (void(HuiEventHandler::*)())&AudioOutput::Update);

	Notify("Play");
	msg_db_l(1);
}

bool AudioOutput::IsPlaying()
{
	return playing;
}

int AudioOutput::GetPos(AudioFile * a)
{
	if ((playing) && (audio == a)){
		int param = 0;
		alGetSourcei(source, AL_SOURCE_STATE, &param);
		TestError("alGetSourcei1 (getpos)");
		if (param == AL_PLAYING){
			alGetSourcei(source, AL_SAMPLE_OFFSET, &param);
			TestError("alGetSourcei2 (getpos)");
			pos = start + stream_pos_0 + param;
			return pos;
		}
	}
	return -1;
}

float AudioOutput::GetVolume()
{
	return volume;
}

void AudioOutput::SetVolume(float _volume)
{
	volume = _volume;
}

void AudioOutput::GetPeaks(float &peak_r, float &peak_l)
{
	peak_r = peak_l = 0;

	if (playing){

		// (sample) position within current stream/buffer
		int dpos = 0;
		alGetSourcei(source, AL_SAMPLE_OFFSET, &dpos);

		// translate relative to data
		int pos_0 = dpos + stream_pos - AL_BUFFER_SIZE * 2;
		if (pos_0 < 0)
			pos_0 = 0;

		// data...
		int num_samples = 2000;
		Array<short> tmp = data.sub(pos_0 * 2, num_samples * 2);

		for (int i=0;i<tmp.num/2;i++){
			float amp_r = fabs((float)tmp[i * 2    ] / 32768.0f);
			float amp_l = fabs((float)tmp[i * 2 + 1] / 32768.0f);
			if (amp_r > peak_r)
				peak_r = amp_r;
			if (amp_l > peak_l)
				peak_l = amp_l;
		}
	}
}

bool AudioOutput::TestError(const string &msg)
{
	int error = alGetError();
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

bool AudioOutput::TestError2(const string &msg, void *d)
{
	int error = alcGetError((ALCdevice*)d);
	if (error != AL_NO_ERROR){
		al_last_error = error;
		msg_error("OpenAL operation: " + msg);
		msg_write(ALError(error));
		//msg_write(alutGetErrorString(error));
		return true;
	}
	return false;
}

void AudioOutput::Update()
{
	msg_db_r("Out.Update", 1);
	TestError("idle");
	if (playing){
		alSourcef(source, AL_GAIN, volume);
		TestError("alGetSourcef(volume) (idle)");
		//msg_write("..");
		int processed;
		alGetSourcei(source, AL_BUFFERS_PROCESSED, &processed);
		TestError("alGetSourcei(processed) (idle)");
		while(processed--){
			stream_pos_0 += AL_BUFFER_SIZE;
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
		if (param != AL_PLAYING){
			//msg_write("hat gestoppt...");
			if (loop)
				Play(audio, true);
			else
				Stop();
		}else{
		}

		HuiRunLaterM(UPDATE_TIME, this, (void(HuiEventHandler::*)())&AudioOutput::Update);
	}
	msg_db_l(1);
}

