/*
 * AudioRenderer.cpp
 *
 *  Created on: 26.03.2012
 *      Author: michi
 */

#include "AudioRenderer.h"

AudioRenderer::AudioRenderer()
{
}

void AudioRenderer::__init__()
{
	new(this) AudioRenderer;
}

void AudioRenderer::__delete__()
{
}
