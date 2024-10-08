/*
 * Beat.h
 *
 *  Created on: 07.10.2017
 *      Author: michi
 */

#ifndef SRC_DATA_RHYTHM_BEAT_H_
#define SRC_DATA_RHYTHM_BEAT_H_

#include "../Range.h"


namespace tsunami {

class Beat {
public:
	Beat() {}
	Beat(const Range &r, int beat, int level, int bar_index, int bar_no);
	Range range;
	int bar_index;
	int bar_no;
	int beat_no;
	int level;
	Range sub(int index, int parts);
	Beat operator+(int offset) const;
	Beat operator-(int offset) const;
	string str() const;
};

}


#endif /* SRC_DATA_RHYTHM_BEAT_H_ */
