/*
 * Bar.h
 *
 *  Created on: 07.10.2017
 *      Author: michi
 */

#ifndef SRC_RHYTHM_BAR_H_
#define SRC_RHYTHM_BAR_H_

#include "../Data/Range.h"


class Bar
{
public:
	Bar(){}
	Bar(const Range &r, int num_beats, int index);
	Range range;
	int num_beats;
	int index;
	float bpm(float sample_rate);
};



#endif /* SRC_RHYTHM_BAR_H_ */
