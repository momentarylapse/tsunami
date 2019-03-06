/*
 * BufferStreamer.h
 *
 *  Created on: 25.11.2015
 *      Author: michi
 */

#ifndef SRC_MODULE_AUDIO_BUFFERSTREAMER_H_
#define SRC_MODULE_AUDIO_BUFFERSTREAMER_H_

#include "AudioSource.h"

class BufferStreamer : public AudioSource
{
public:
	BufferStreamer(AudioBuffer *buf);

	void _cdecl __init__(AudioBuffer *buf);
	void _cdecl __delete__() override;

	int read(AudioBuffer &buf) override;
	void reset() override;
	int get_pos() override { return offset; }
	void set_pos(int pos) override { offset = pos; }

	AudioBuffer *buf;
	int offset;
};

#endif /* SRC_MODULE_AUDIO_BUFFERSTREAMER_H_ */
