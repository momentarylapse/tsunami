/*
 * EffectRenderer.cpp
 *
 *  Created on: 28.04.2016
 *      Author: michi
 */

#include "EffectRenderer.h"
#include "../../Plugins/Effect.h"

EffectRenderer::EffectRenderer(AudioRenderer* _child, const Array<Effect*> _fx)
{
	child = _child;
	fx = _fx;
	track = NULL;
}

EffectRenderer::~EffectRenderer()
{
}

void EffectRenderer::__init__(AudioRenderer *_child, const Array<Effect*> _fx)
{
	new(this) EffectRenderer(_child, _fx);
}

void EffectRenderer::__delete__()
{
	fx.clear();
}

int EffectRenderer::read(BufferBox& buf)
{
	int r = child->read(buf);
	foreach(Effect *f, fx){
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

Range EffectRenderer::range()
{
	return child->range();
}

int EffectRenderer::getPos()
{
	return child->getPos();
}

void EffectRenderer::seek(int pos)
{
	child->seek(pos);
}

int EffectRenderer::getSampleRate()
{
	return child->getSampleRate();
}

int EffectRenderer::getNumSamples()
{
	return child->getNumSamples();
}

Array<Tag> EffectRenderer::getTags()
{
	return child->getTags();
}

void EffectRenderer::setTrack(Track *t)
{
	track = t;
}
