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

	virtual int _cdecl read(BufferBox &buf);
	virtual void _cdecl reset();
	virtual Range _cdecl range();
	virtual int _cdecl getPos(){ return offset; }
	virtual void _cdecl seek(int pos);
	//virtual int _cdecl getSampleRate(){ return DEFAULT_SAMPLE_RATE; }
	virtual int _cdecl getNumSamples();

	BufferBox *buf;
	int offset;
};

#endif /* SRC_AUDIO_RENDERER_BUFFERRENDERER_H_ */
