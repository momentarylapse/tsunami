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
	int _cdecl read(Array<Beat> &beats, int samples) override;
	void _cdecl reset() override;
	void seek(int pos);

	int _cdecl beats_per_bar() override;
	int _cdecl cur_beat() override;
	int _cdecl cur_bar() override;
	float _cdecl beat_fraction() override;

	BarCollection bars;
	int offset;
};



#endif /* SRC_MODULE_BEATS_BARSTREAMER_H_ */
