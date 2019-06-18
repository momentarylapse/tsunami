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

	int position;
	int target;
	int samples;
	Range range();
};

#endif /* SRC_DATA_CROSSFADE_H_ */
