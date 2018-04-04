/*
 * Beat.h
 *
 *  Created on: 07.10.2017
 *      Author: michi
 */

#ifndef SRC_DATA_RHYTHM_BEAT_H_
#define SRC_DATA_RHYTHM_BEAT_H_

#include "../Range.h"


class Beat
{
public:
	Beat(){}
	Beat(const Range &r, int bar, int beat, int level);
	Range range;
	int bar_no;
	int beat_no;
	int level;
	Range sub(int index, int parts);
};


#endif /* SRC_DATA_RHYTHM_BEAT_H_ */
