/*
 * AudioSource.h
 *
 *  Created on: 26.03.2012
 *      Author: michi
 */

#ifndef SRC_AUDIO_SOURCE_AUDIOSOURCE_H_
#define SRC_AUDIO_SOURCE_AUDIOSOURCE_H_

#include "../../Data/Song.h"

class AudioBuffer;

class AudioSource : public VirtualBase
{
public:
	AudioSource();
	virtual ~AudioSource(){}

	void _cdecl __init__();
	virtual void _cdecl __delete__();

	virtual int _cdecl read(AudioBuffer &buf){ return 0; }
	virtual void _cdecl reset(){}

	virtual int _cdecl getSampleRate(){ return DEFAULT_SAMPLE_RATE; }

	static const int END_OF_STREAM;
};

#endif /* SRC_AUDIO_SOURCE_AUDIOSOURCE_H_ */
