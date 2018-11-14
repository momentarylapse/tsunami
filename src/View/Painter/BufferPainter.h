/*
 * BufferPainter.h
 *
 *  Created on: 13.11.2018
 *      Author: michi
 */

#ifndef SRC_VIEW_PAINTER_BUFFERPAINTER_H_
#define SRC_VIEW_PAINTER_BUFFERPAINTER_H_

class AudioView;

class BufferPainter
{
public:
	BufferPainter(AudioView *view);

	AudioView *view;
};

#endif /* SRC_VIEW_PAINTER_BUFFERPAINTER_H_ */
