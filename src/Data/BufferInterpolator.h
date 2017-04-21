/*
 * BufferInterpolator.h
 *
 *  Created on: 21.04.2017
 *      Author: michi
 */

#ifndef SRC_DATA_BUFFERINTERPOLATOR_H_
#define SRC_DATA_BUFFERINTERPOLATOR_H_

class BufferBox;

class BufferInterpolator
{
public:
	BufferInterpolator();
	~BufferInterpolator();

	enum Method{
		METHOD_LINEAR,
		METHOD_CUBIC,
		METHOD_SINC,
		METHOD_FOURIER,
	};

	static void interpolate(BufferBox &in, BufferBox &out, int new_size, Method method);
};

#endif /* SRC_DATA_BUFFERINTERPOLATOR_H_ */
