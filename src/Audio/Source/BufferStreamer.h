/*
 * BufferStreamer.h
 *
 *  Created on: 25.11.2015
 *      Author: michi
 */

#ifndef SRC_AUDIO_SOURCE_BUFFERSTREAMER_H_
#define SRC_AUDIO_SOURCE_BUFFERSTREAMER_H_

#include "../Source/AudioSource.h"

class BufferStreamer : public AudioSource
{
public:
	BufferStreamer(AudioBuffer *buf);
	virtual ~BufferStreamer(){}

	void _cdecl __init__(AudioBuffer *buf);
	virtual void _cdecl __delete__();

	virtual int _cdecl read(AudioBuffer &buf);
	virtual void _cdecl reset();
	int _cdecl getPos(){ return offset; }
	void _cdecl seek(int pos);
	//virtual int _cdecl getSampleRate(){ return DEFAULT_SAMPLE_RATE; }

	AudioBuffer *buf;
	int offset;
};

#endif /* SRC_AUDIO_SOURCE_BUFFERSTREAMER_H_ */
