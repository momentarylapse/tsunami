/*
 * CrossFade.h
 *
 *  Created on: 06.09.2018
 *      Author: michi
 */

#ifndef SRC_DATA_CROSSFADE_H_
#define SRC_DATA_CROSSFADE_H_

#include "Range.h"

class CrossFade {
public:

	enum Mode {
		INWARD,
		OUTWARD
	};

	int position;
	Mode mode;
	int samples;
	Range range();
};

class CrossFadeOld {
public:
	int position;
	int target;
	int samples;
};

#endif /* SRC_DATA_CROSSFADE_H_ */
