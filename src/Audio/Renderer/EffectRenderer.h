/*
 * EffectRenderer.h
 *
 *  Created on: 28.04.2016
 *      Author: michi
 */

#ifndef SRC_AUDIO_RENDERER_EFFECTRENDERER_H_
#define SRC_AUDIO_RENDERER_EFFECTRENDERER_H_

#include "AudioRenderer.h"

class Effect;
class Track;

class EffectRenderer : public AudioRenderer
{
public:
	EffectRenderer(AudioRenderer *child, const Array<Effect*> fx);
	virtual ~EffectRenderer();

	void _cdecl __init__(AudioRenderer *child, const Array<Effect*> fx);
	virtual void _cdecl __delete__();

	virtual int read(BufferBox &buf);
	virtual void reset();
	virtual Range range();
	virtual int getPos();
	virtual void seek(int pos);
	virtual int getSampleRate();
	virtual int getNumSamples();
	virtual Array<Tag> getTags();

	void setTrack(Track *t);

	AudioRenderer *child;
	Array<Effect*> fx;
	Track *track;
};

#endif /* SRC_AUDIO_RENDERER_EFFECTRENDERER_H_ */
