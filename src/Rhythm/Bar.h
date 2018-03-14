/*
 * Bar.h
 *
 *  Created on: 07.10.2017
 *      Author: michi
 */

#ifndef SRC_RHYTHM_BAR_H_
#define SRC_RHYTHM_BAR_H_

#include "../Data/Range.h"

class BarCollection;

class BarPattern
{
public:
	BarPattern();
	BarPattern(int length, int num_beats, int num_sub_beats);
	int length;
	int num_beats;
	int num_sub_beats;

	enum{
		TYPE_BAR,
		TYPE_PAUSE
	};
};

class Bar : public BarPattern
{
public:
	Bar(){}
	Bar(int length, int num_beats, int num_sub_beats);
	bool is_pause();
	float bpm(float sample_rate);


	// filled by BarCollection.getBars()
	int index; // index in the Bar[] array
	int index_text; // index without pause "bars" (well, text = n+1...)
	int offset;
	Range range();
};



#endif /* SRC_RHYTHM_BAR_H_ */
