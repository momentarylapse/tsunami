/*
 * TrackMarker.cpp
 *
 *  Created on: 17.03.2019
 *      Author: michi
 */

#include "TrackMarker.h"
#include "Midi/Scale.h"

TrackMarker::TrackMarker() {}

TrackMarker::TrackMarker(const Range &r, const string &t) {
	range = r;
	text = t;
}

string TrackMarker::nice_text() const {
	if (marker_is_key(text)) {
		Scale s = parse_marker_key(text);
		return s.nice_name();
	}
	return text;
}


bool marker_is_key(const string &text) {
	return text.match("::key=*::");
}

Scale parse_marker_key(const string &text) {
	Scale scale = Scale::C_MAJOR;
	if (!marker_is_key(text))
		return scale;
	return Scale::parse(text.substr(6, -3));
}

