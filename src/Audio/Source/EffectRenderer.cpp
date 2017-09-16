/*
 * EffectRenderer.cpp
 *
 *  Created on: 28.04.2016
 *      Author: michi
 */

#include "../Source/EffectRenderer.h"

#include "../../Plugins/Effect.h"

EffectRenderer::EffectRenderer(AudioSource* _child, const Array<Effect*> _fx)
{
	child = _child;
	fx = _fx;
	track = NULL;
}

EffectRenderer::~EffectRenderer()
{
}

void EffectRenderer::__init__(AudioSource *_child, const Array<Effect*> _fx)
{
	new(this) EffectRenderer(_child, _fx);
}

void EffectRenderer::__delete__()
{
	this->EffectRenderer::~EffectRenderer();
}

int EffectRenderer::read(AudioBuffer& buf)
{
	int r = child->read(buf);
	for (Effect *f : fx){
		if (!f->enabled)
			continue;
		f->apply(buf, track, false);
	}
	return r;
}

void EffectRenderer::reset()
{
	child->reset();
}

int EffectRenderer::getSampleRate()
{
	return child->getSampleRate();
}

void EffectRenderer::setTrack(Track *t)
{
	track = t;
}
