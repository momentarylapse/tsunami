/*
 * TrackMarker.h
 *
 *  Created on: 06.09.2018
 *      Author: michi
 */

#ifndef SRC_DATA_TRACKMARKER_H_
#define SRC_DATA_TRACKMARKER_H_

#include "Range.h"

class AudioEffect;
class Scale;


class TrackMarker {
public:
	Range range;
	string text;
	Array<AudioEffect*> fx;

	string nice_text() const;
};


bool marker_is_key(const string &text);
Scale parse_marker_key(const string &text);



#endif /* SRC_DATA_TRACKMARKER_H_ */
