/*
 * ExtendedBufferBox.h
 *
 *  Created on: 01.02.2013
 *      Author: michi
 */

#ifndef EXTENDEDBUFFERBOX_H_
#define EXTENDEDBUFFERBOX_H_

#include "../Data/BufferBox.h"
class complex;

class ExtendedBufferBox : public BufferBox
{
public:

	void get_spectrum(Array<complex> &spec_r, Array<complex> &spec_l, int samples);
};

#endif /* EXTENDEDBUFFERBOX_H_ */
