/*
 * AudioStream.h
 *
 *  Created on: 01.11.2014
 *      Author: michi
 */

#ifndef SRC_AUDIO_AUDIOSTREAM_H_
#define SRC_AUDIO_AUDIOSTREAM_H_



#include "../lib/base/base.h"
#include "../lib/hui/hui.h"
#include "../Data/AudioFile.h"
#include "../View/Helper/PeakMeter.h"

class AudioRendererInterface;
class AudioOutput;
typedef void PaStream;

class AudioStream : public PeakMeterSource
{
public:
	AudioStream();
	virtual ~AudioStream();

	void _cdecl __init__();
	void _cdecl __delete__();

	void kill();


	static const string MESSAGE_STATE_CHANGE;
	static const string MESSAGE_UPDATE;


	void _cdecl stop();
	void _cdecl play();
	void _cdecl pause();
	void _cdecl update();

	bool _cdecl isPlaying();
	bool _cdecl isPaused();
	int _cdecl getState();
	void _cdecl setSource(AudioRendererInterface *r);
	void _cdecl setSourceGenerated(void *func, int sample_rate);
	AudioRendererInterface *getSource(){	return renderer;	}
	int getPos();
	bool getPosSafe(int &pos);
	void flushBuffers();

	float getSampleRate();
	void getSomeSamples(BufferBox &buf, int num_samples);

	float getVolume();
	void setVolume(float _volume);

	void setBufferSize(int _size){	buffer_size = _size;	}

private:
	bool testError(const string &msg);
	bool stream(int buf);

	void stop_play();
	void start_play(int pos);


	float volume;
	bool playing;
	int sample_rate;
	int buffer_size;

	AudioRendererInterface *renderer;
	BufferBox box[2];

	typedef int generate_func_t(BufferBox &);
	generate_func_t *generate_func;

	Array<short> data;
	int data_samples;

	PaStream *pa_stream;

	int last_error;

	AudioOutput *output;
	bool killed;
};

#endif /* SRC_AUDIO_AUDIOSTREAM_H_ */
