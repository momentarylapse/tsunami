/*
 * BufferRenderer.h
 *
 *  Created on: 25.11.2015
 *      Author: michi
 */

#ifndef SRC_AUDIO_RENDERER_BUFFERRENDERER_H_
#define SRC_AUDIO_RENDERER_BUFFERRENDERER_H_

#include "AudioRenderer.h"

class BufferRenderer : public AudioRenderer
{
public:
	BufferRenderer(BufferBox *buf);
	virtual ~BufferRenderer(){}

	void _cdecl __init__(BufferBox *buf);
	virtual void _cdecl __delete__();

	virtual int read(BufferBox &buf);
	virtual void reset();
	virtual Range range();
	virtual int getPos(){ return offset; }
	virtual void seek(int pos);
	//virtual int getSampleRate(){ return DEFAULT_SAMPLE_RATE; }
	virtual int getNumSamples();

	BufferBox *buf;
	int offset;
};

#endif /* SRC_AUDIO_RENDERER_BUFFERRENDERER_H_ */
