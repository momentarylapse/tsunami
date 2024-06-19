/*
 * TrackMarker.cpp
 *
 *  Created on: 17.03.2019
 *      Author: michi
 */

#include "TrackMarker.h"
#include "midi/Scale.h"
#include "../lib/os/msg.h"

namespace tsunami {

TrackMarker::TrackMarker() {
	//msg_write("new TrackMarker " + p2s(this));
}

TrackMarker::TrackMarker(const Range &r, const string &t) {
	//msg_write("new TrackMarker " + p2s(this));
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

// TODO fx
TrackMarker *TrackMarker::copy(int offset) const {
	if (fx.num > 0)
		msg_error("TODO: copy fx");
	return new TrackMarker(range + offset, text);
}


bool marker_is_key(const string &text) {
	return text.match("::key=*::");
}

Scale parse_marker_key(const string &text) {
	Scale scale = Scale::C_MAJOR;
	if (!marker_is_key(text))
		return scale;
	return Scale::parse(text.sub(6, -2));
}

}

