/*
 * CrossFade.h
 *
 *  Created on: 06.09.2018
 *      Author: michi
 */

#ifndef SRC_DATA_CROSSFADE_H_
#define SRC_DATA_CROSSFADE_H_

#include "Range.h"


namespace tsunami {

struct CrossFade {
	enum Mode {
		Inward,
		Outward
	};

	int position;
	Mode mode;
	int samples;

	Range range() const;
	bool operator==(const CrossFade &o) const;
	bool operator!=(const CrossFade &o) const;
};

struct CrossFadeLegacy {
	int position;
	int target;
	int samples;
};

}

#endif /* SRC_DATA_CROSSFADE_H_ */
