/*
 * TrackMarker.h
 *
 *  Created on: 06.09.2018
 *      Author: michi
 */

#ifndef SRC_DATA_TRACKMARKER_H_
#define SRC_DATA_TRACKMARKER_H_

#include "Range.h"
#include "../Module/Audio/AudioEffect.h"

class AudioEffect;
class Scale;


class TrackMarker : public Sharable<Empty>{
public:
	TrackMarker();
	TrackMarker(const Range &r, const string &t);
	Range range;
	string text;
	shared_array<AudioEffect> fx;

	string nice_text() const;
	TrackMarker *copy(int offset=0) const;
};


bool marker_is_key(const string &text);
Scale parse_marker_key(const string &text);



#endif /* SRC_DATA_TRACKMARKER_H_ */
