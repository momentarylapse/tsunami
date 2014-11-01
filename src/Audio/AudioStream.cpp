/*
 * AudioStream.cpp
 *
 *  Created on: 01.11.2014
 *      Author: michi
 */

#include "AudioStream.h"
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

#else
	#include <AL/al.h>
	#include <AL/alut.h>
	#include <AL/alc.h>
#endif

//#define DEFAULT_BUFFER_SIZE		131072
#define DEFAULT_BUFFER_SIZE		32768
//#define DEFAULT_BUFFER_SIZE		16384

#define UPDATE_TIME		0.050f


const string AudioStream::MESSAGE_STATE_CHANGE = "StateChange";
const string AudioStream::MESSAGE_UPDATE = "Update";

AudioStream::AudioStream() :
	PeakMeterSource("AudioStream")
{
	al_last_error = AL_NO_ERROR;

	renderer = NULL;
	generate_func = NULL;

	playing = false;
	volume = 1;

	output = tsunami->output;

	buffer[0] = -1;
	buffer[1] = -1;
	source = -1;
	data_samples = 0;
	buffer_size = 0;
	cur_buffer_no = 0;
	sample_rate = DEFAULT_SAMPLE_RATE;

	alGenSources(1, &source);

	alGenBuffers(2, (ALuint*)buffer);
	testError("alGenBuffers (play)");
}

AudioStream::~AudioStream()
{
	stop();

	alDeleteBuffers(2, (ALuint*)buffer);
	testError("alDeleteBuffers (stop)");
}

void AudioStream::__init__()
{
	new(this) AudioStream;
}

void AudioStream::__delete__()
{
	this->~AudioStream();
}

void AudioStream::stop_play()
{
	if (!playing)
		return;

	testError("?  (prestop)");
	alSourceStop(source);
	testError("alSourceStop (stop)");
	int queued;
	alGetSourcei(source, AL_BUFFERS_QUEUED, &queued);
	testError("alGetSourcei(queued) (stop)");
	while(queued--){
		ALuint buf;
		alSourceUnqueueBuffers(source, 1, &buf);
		testError(format("alSourceUnqueueBuffers(%d) (stop)", queued));
	}
}

void AudioStream::stop()
{
	if (!playing)
		return;
	msg_db_f("Output.Stop", 1);

	stop_play();

	playing = false;
	renderer = NULL;

	notify(MESSAGE_STATE_CHANGE);
}

void AudioStream::pause()
{
	if (!playing)
		return;
	int param;
	alGetSourcei(source, AL_SOURCE_STATE, &param);
	if (param == AL_PLAYING)
		alSourcePause(source);
	else if (param == AL_PAUSED)
		alSourcePlay(source);
	notify(MESSAGE_STATE_CHANGE);
}

bool AudioStream::stream(int buf)
{
	msg_db_f("stream", 1);

	BufferBox *b = (buf == buffer[0]) ? &box[0] : &box[1];
	int size = 0;
	b->resize(buffer_size);
	//b->offset = stream_offset_next;
	if (renderer){
		size = renderer->read(*b);
		//msg_write(size);
	}else if (generate_func){
		size = (*generate_func)(*b);
	}
	if (size == 0)
		return false;
	//b->offset = stream_offset_next;
	b->get_16bit_buffer(data);
	alBufferData(buf, AL_FORMAT_STEREO16, &data[0], size * 4, sample_rate);
	testError("alBufferData (stream)");

	return true;
}

void AudioStream::start_play(int pos)
{
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
	if (testError("alSourcef... (play)"))
		return;

	cur_buffer_no = 0;
	alSourceQueueBuffers(source, num_buffers, (ALuint*)buffer);
	testError("alSourceQueueBuffers (play)");

	alSourcePlay(source);
	if (testError("alSourcePlay (play)"))
		return;
}

void AudioStream::setSource(AudioRenderer *r)
{
	msg_db_f("Stream.setSource", 1);

	if (playing)
		stop();

	renderer = r;
	generate_func = NULL;
	sample_rate = tsunami->audio->sample_rate; //r->sample_rate; // FIXME
}

void AudioStream::setSourceGenerated(void *func, int _sample_rate)
{
	msg_db_f("Stream.setSourceGen", 1);

	if (playing)
		stop();

	renderer = NULL;
	generate_func = (generate_func_t*)func;
	sample_rate = _sample_rate;
}

void AudioStream::play()
{
	msg_db_f("Stream.play", 1);

	if (playing)
		stop();

	start_play(0);

	playing = true;

	HuiRunLaterM(UPDATE_TIME, this, &AudioStream::update);

	notify(MESSAGE_STATE_CHANGE);
}

bool AudioStream::isPlaying()
{
	if (!playing)
		return false;
	int param = 0;
	alGetSourcei(source, AL_SOURCE_STATE, &param);
	testError("alGetSourcei1 (getpos)");
	return ((param == AL_PLAYING) or (param == AL_PAUSED));
}

int AudioStream::getState()
{
	/*if (IsPlaying())
		return STATE_PLAYING;*/
	int param;
	alGetSourcei(source, AL_SOURCE_STATE, &param);
	if (param == AL_PLAYING)
		return STATE_PLAYING;
	if (param == AL_PAUSED)
		return STATE_PAUSED;
	return STATE_STOPPED;
}

int AudioStream::getPos()
{
	int pos;
	if (getPosSafe(pos))
		return pos;
	return 0;
}

bool AudioStream::getPosSafe(int &pos)
{
	if (playing){
		int param = 0;
		alGetSourcei(source, AL_SOURCE_STATE, &param);
		testError("alGetSourcei1 (getpos)");
		if ((param == AL_PLAYING) or (param == AL_PAUSED)){
			alGetSourcei(source, AL_SAMPLE_OFFSET, &param);
			testError("alGetSourcei2 (getpos)");
			pos = box[cur_buffer_no].offset + param;
			return true;
		}
	}
	return false;
}

float AudioStream::getVolume()
{
	return volume;
}

void AudioStream::setVolume(float _volume)
{
	volume = _volume;
	notify(MESSAGE_STATE_CHANGE);
}

float AudioStream::getSampleRate()
{
	return sample_rate;
}

void AudioStream::getSomeSamples(BufferBox &buf, int num_samples)
{
	if (!playing)
		return;

	// (sample) position within current stream/buffer
	int dpos = 0;
	alGetSourcei(source, AL_SAMPLE_OFFSET, &dpos);

	buf.set_as_ref(box[cur_buffer_no], dpos, min(num_samples, box[cur_buffer_no].num - dpos));
}

bool AudioStream::testError(const string &msg)
{
	return output->testError(msg);
}

void AudioStream::update()
{
	msg_db_f("Stream.update", 1);
	testError("idle");
	if (playing){
		alSourcef(source, AL_GAIN, volume);
		testError("alGetSourcef(volume) (idle)");
		//msg_write("..");
		int processed;
		alGetSourcei(source, AL_BUFFERS_PROCESSED, &processed);
		testError("alGetSourcei(processed) (idle)");
		while(processed--){
			cur_buffer_no = 1 - cur_buffer_no;
			ALuint buf;
			alSourceUnqueueBuffers(source, 1, &buf);
			testError("alSourceUnqueueBuffers (idle)");
			if (stream(buf)){
				alSourceQueueBuffers(source, 1, &buf);
				testError("alSourceQueueBuffers (idle)");
			}
		}
		notify(MESSAGE_UPDATE);


		int param=0;
		alGetSourcei(source,AL_SOURCE_STATE, &param);
		testError("alGetSourcei(state) (idle)");
		if ((param != AL_PLAYING) and (param != AL_PAUSED)){
			//msg_write("hat gestoppt...");
				stop();
		}else{
		}

		if (buffer_size > DEFAULT_BUFFER_SIZE / 3)
			HuiRunLaterM(UPDATE_TIME, this, &AudioStream::update);
		else
			HuiRunLaterM(0, this, &AudioStream::update);
	}
}


