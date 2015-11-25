/*
 * BufferRenderer.cpp
 *
 *  Created on: 25.11.2015
 *      Author: michi
 */

#include "BufferRenderer.h"

BufferRenderer::BufferRenderer(BufferBox *b)
{
	buf = b;
	offset = 0;
}

void BufferRenderer::__init__(BufferBox* buf)
{
	new(this) BufferRenderer(buf);
}

void BufferRenderer::__delete__()
{
}

int BufferRenderer::read(BufferBox& _buf)
{
	int n = min(_buf.num, buf->num - offset);
	_buf.set(*buf, -offset, 1);
	offset += n;
	return n;
}

void BufferRenderer::reset()
{
	offset = 0;
}

Range BufferRenderer::range()
{
	return Range(0, buf->num);
}

void BufferRenderer::seek(int pos)
{
	offset = pos;
}

int BufferRenderer::getNumSamples()
{
	return buf->num;
}
