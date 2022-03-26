/*
 * BarStreamer.h
 *
 *  Created on: 07.10.2017
 *      Author: michi
 */

#ifndef SRC_MODULE_BEATS_BARSTREAMER_H_
#define SRC_MODULE_BEATS_BARSTREAMER_H_

#include "BeatSource.h"
#include "../../data/rhythm/BarCollection.h"

class BarStreamer : public BeatSource {
public:
	BarStreamer(BarCollection &bars);
	int read(Array<Beat> &beats, int samples) override;
	void reset() override;
	void set_pos(int pos) override;
	int get_pos() override;

	int beats_per_bar() override;
	int cur_beat() override;
	int cur_bar() override;
	float beat_fraction() override;

	BarCollection bars;
	int offset;
};



#endif /* SRC_MODULE_BEATS_BARSTREAMER_H_ */
