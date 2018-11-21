/*
 * BufferPainter.h
 *
 *  Created on: 13.11.2018
 *      Author: michi
 */

#ifndef SRC_VIEW_PAINTER_BUFFERPAINTER_H_
#define SRC_VIEW_PAINTER_BUFFERPAINTER_H_

#include "../../lib/base/base.h"
#include "../../lib/math/rect.h"
#include "../../lib/image/color.h"

class AudioView;
class Painter;
class AudioBuffer;
class color;
class Range;

class BufferPainter
{
public:
	BufferPainter(AudioView *view);


	void draw_buffer(Painter *c, AudioBuffer &b, int offset);
	void draw_buffer_selection(Painter *c, AudioBuffer &b, int offset);

	void set_context(const rect &area);
	void set_color(const color &col);
	void set_clip(const Range &r);

	AudioView *view;
	rect area;
	color col;
	float x0, x1;
};

#endif /* SRC_VIEW_PAINTER_BUFFERPAINTER_H_ */
