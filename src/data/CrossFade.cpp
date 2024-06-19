/*
 * CrossFade.cpp
 *
 *  Created on: 06.09.2018
 *      Author: michi
 */

#include "CrossFade.h"


namespace tsunami {

Range CrossFade::range() const {
	return Range(position, samples);
}

bool CrossFade::operator ==(const CrossFade &o) const {
	return (position == o.position) and (mode == o.mode) and (samples == o.samples);
}

bool CrossFade::operator !=(const CrossFade &o) const {
	return !(*this == o);
}

}
