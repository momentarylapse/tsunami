/*
 * EffectRenderer.h
 *
 *  Created on: 28.04.2016
 *      Author: michi
 */

#ifndef SRC_AUDIO_SOURCE_EFFECTRENDERER_H_
#define SRC_AUDIO_SOURCE_EFFECTRENDERER_H_

#include "../Source/AudioSource.h"

class Effect;
class Track;

class EffectRenderer : public AudioSource
{
public:
	EffectRenderer(AudioSource *child, const Array<Effect*> fx);
	virtual ~EffectRenderer();

	void _cdecl __init__(AudioSource *child, const Array<Effect*> fx);
	virtual void _cdecl __delete__();

	virtual int _cdecl read(AudioBuffer &buf);
	virtual void _cdecl reset();
	virtual int _cdecl getSampleRate();

	void setTrack(Track *t);

	AudioSource *child;
	Array<Effect*> fx;
	Track *track;
};

#endif /* SRC_AUDIO_SOURCE_EFFECTRENDERER_H_ */
