/*
 * AudioJoiner.h
 *
 *  Created on: 02.04.2018
 *      Author: michi
 */

#ifndef SRC_AUDIO_AUDIOJOINER_H_
#define SRC_AUDIO_AUDIOJOINER_H_

#include "Source/AudioSource.h"

class AudioJoiner : public AudioSource
{
public:
	AudioJoiner(AudioSource *a, AudioSource *b);
	virtual ~AudioJoiner();

	virtual int _cdecl read(AudioBuffer &buf);
	virtual void _cdecl reset();
	virtual int _cdecl get_pos(int delta);
	virtual int _cdecl sample_rate();

	AudioSource *a, *b;
	void _cdecl set_source_a(AudioSource *a);
	void _cdecl set_source_b(AudioSource *b);
};

#endif /* SRC_AUDIO_AUDIOJOINER_H_ */
