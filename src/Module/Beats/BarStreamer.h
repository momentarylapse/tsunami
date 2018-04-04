/*
 * BarStreamer.h
 *
 *  Created on: 07.10.2017
 *      Author: michi
 */

#ifndef SRC_MODULE_BEATS_BARSTREAMER_H_
#define SRC_MODULE_BEATS_BARSTREAMER_H_

#include "BeatSource.h"
#include "../../Data/Rhythm/BarCollection.h"

class BarStreamer : public BeatSource
{
public:
	BarStreamer(BarCollection &bars);
	virtual int _cdecl read(Array<Beat> &beats, int samples);
	virtual void _cdecl reset();
	void seek(int pos);

	BarCollection bars;
	int offset;
};



#endif /* SRC_MODULE_BEATS_BARSTREAMER_H_ */
