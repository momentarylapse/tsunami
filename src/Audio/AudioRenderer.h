/*
 * AudioRenderer.h
 *
 *  Created on: 26.03.2012
 *      Author: michi
 */

#ifndef AUDIORENDERER_H_
#define AUDIORENDERER_H_

#include "../Data/Song.h"

class AudioRenderer : public VirtualBase
{
public:
	AudioRenderer();
	virtual ~AudioRenderer(){}

	void _cdecl __init__();
	virtual void _cdecl __delete__();

	virtual int read(BufferBox &buf){ return 0; }
	virtual void reset(){}
	virtual Range range(){ return Range(0, 0); }
	virtual int getPos(){ return 0; }
	virtual void seek(int pos){}
	virtual int getSampleRate(){ return DEFAULT_SAMPLE_RATE; }
};

#endif /* AUDIORENDERER_H_ */
