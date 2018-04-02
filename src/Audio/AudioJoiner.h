/*
 * AudioJoiner.h
 *
 *  Created on: 02.04.2018
 *      Author: michi
 */

#ifndef SRC_AUDIO_AUDIOJOINER_H_
#define SRC_AUDIO_AUDIOJOINER_H_

#include "Source/AudioPort.h"

class AudioJoiner : public AudioPort
{
public:
	AudioJoiner(AudioPort *a, AudioPort *b);
	virtual ~AudioJoiner();

	virtual int _cdecl read(AudioBuffer &buf);
	virtual void _cdecl reset();
	virtual int _cdecl get_pos(int delta);
	virtual int _cdecl sample_rate();

	AudioPort *a, *b;
	void _cdecl set_source_a(AudioPort *a);
	void _cdecl set_source_b(AudioPort *b);
};

#endif /* SRC_AUDIO_AUDIOJOINER_H_ */
