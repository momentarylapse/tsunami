/*
 * AudioPort.h
 *
 *  Created on: 26.03.2012
 *      Author: michi
 */

#ifndef SRC_AUDIO_SOURCE_AUDIOPORT_H_
#define SRC_AUDIO_SOURCE_AUDIOPORT_H_

#include "../../Data/Song.h"

class AudioBuffer;

class AudioPort : public VirtualBase
{
public:
	AudioPort();
	virtual ~AudioPort(){}

	void _cdecl __init__();
	virtual void _cdecl __delete__();

	virtual int _cdecl read(AudioBuffer &buf){ return 0; }
	virtual void _cdecl reset(){}
	virtual int _cdecl get_pos(int delta){ return -1; }
	virtual int _cdecl sample_rate(){ return DEFAULT_SAMPLE_RATE; }

	static const int END_OF_STREAM;
	static const int NOT_ENOUGH_DATA;
};

#endif /* SRC_AUDIO_SOURCE_AUDIOPORT_H_ */
