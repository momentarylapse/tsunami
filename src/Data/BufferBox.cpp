/*
 * BufferBox.cpp
 *
 *  Created on: 21.03.2012
 *      Author: michi
 */

#include "BufferBox.h"
#include "../Tsunami.h"


BufferBox::BufferBox()
{
	//HistoryStructReset("BufferBox", this);

	offset = 0;
	num = 0;
}

BufferBox::~BufferBox()
{	clear();	}

void BufferBox::clear()
{
	r.clear();
	l.clear();
	num = 0;
}

void BufferBox::resize(int length)
{
	r.resize(length);
	l.resize(length);
	num = length;
}

bool BufferBox::is_ref()
{	return (r.allocated == 0);	}

void fa_make_own(Array<float> &a)
{
	void *data = a.data;
	int num = a.num;
	a.clear();
	a.resize(num);
	memcpy(a.data, data, a.element_size * num);
}

void BufferBox::make_own()
{
	if (is_ref()){
		//msg_write("bb::make_own!");
		fa_make_own(r);
		fa_make_own(l);
	}
}

void BufferBox::swap(BufferBox &b)
{
	// buffer
	r.exchange(b.r);
	l.exchange(b.l);

	// num
	int t = num;
	num = b.num;
	b.num = t;

	// offset
	t = offset;
	offset = b.offset;
	b.offset = t;
}

void BufferBox::scale(float volume)
{
	if (volume != 1.0f){
		make_own();

		// scale
		for (int i=0;i<r.num;i++){
			r[i] *= volume;
			l[i] *= volume;
		}
	}
}

void BufferBox::add(const BufferBox &b, int offset, float volume)
{
	// relative to b
	int i0 = max(0, -offset);
	int i1 = min(b.r.num, r.num - offset);

	// add buffers
	if (volume == 1.0f){
		for (int i=i0;i<i1;i++){
			r[i + offset] += b.r[i];
			l[i + offset] += b.l[i];
		}
	}else{
		for (int i=i0;i<i1;i++){
			r[i + offset] += b.r[i] * volume;
			l[i + offset] += b.l[i] * volume;
		}
	}
}

void BufferBox::set(const BufferBox &b, int offset, float volume)
{
	// relative to b
	int i0 = max(0, -offset);
	int i1 = min(b.r.num, r.num - offset);
	if (i1 <= i0)
		return;

	// set buffers
	if (volume == 1.0f){
		memcpy(&r[i0 + offset], (float*)b.r.data + i0, sizeof(float) * (i1 - i0));
		memcpy(&l[i0 + offset], (float*)b.l.data + i0, sizeof(float) * (i1 - i0));
	}else{
		for (int i=i0;i<i1;i++){
			r[i + offset] = b.r[i] * volume;
			l[i + offset] = b.l[i] * volume;
		}
	}
}

void BufferBox::set_as_ref(const BufferBox &b, int _offset, int _length)
{
	clear();
	num = _length;
	offset = _offset + b.offset;
	r.set_ref(b.r.sub(_offset, _length));
	l.set_ref(b.l.sub(_offset, _length));
}

void BufferBox::set_16bit(const void *b, int offset, int length)
{
	// relative to b
	int i0 = max(0, - offset);
	int i1 = min(length, num - offset);
	length = i1 - i0;
	float *pr = &r[i0 + offset];
	float *pl = &l[i0 + offset];
	short *pb = &((short*)b)[i0 * 2];
	for (int i=0;i<length;i++){
		(*pr ++) = (float)(*pb ++) / 32768.0f;
		(*pl ++) = (float)(*pb ++) / 32768.0f;
	}
}


#define val_max		32766
#define val_alert	32770

static bool wtb_overflow;

inline void set_data(short *data, float value)
{
	int value_int = (int)(value * 32768.0f);
	if (value_int > val_max){
		if (value_int > val_alert)
			wtb_overflow = true;
		value_int = val_max;
	}else if (value_int < - val_max){
		if (value_int < -val_alert)
			wtb_overflow = true;
		value_int = -val_max;
	}
	*data = value_int;
}

void BufferBox::get_16bit_buffer(Array<short> &data)
{
	wtb_overflow = false;

	data.resize(num * 2);
	short *b = &data[0];
	for (int i=0;i<num;i++){
		set_data(b ++, r[i]);
		set_data(b ++, l[i]);
	}

	if (wtb_overflow)
		tsunami->Log(Tsunami::LOG_ERROR, _("Amplitude zu gro&s, Signal &ubersteuert."));
		//msg_error("overflow");
}
