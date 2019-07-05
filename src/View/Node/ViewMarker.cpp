/*
 * ViewMarker.h
 *
 *  Created on: 04.07.2019
 *      Author: michi
 */
 
 #include "ViewMarker.h"

ViewMarker::ViewMarker(AudioViewTrack *parent, TrackMarker *marker) {
}

ViewMarker::~ViewMarker() {
}


HoverData ViewMarker::get_hover_data_default(float mx, float my) {
	auto s = view->hover_time(mx, my);
	s.vlayer = this;
	s.node = this;
	s.vtrack = view->get_track(layer->track);
	s.type = HoverData::Type::LAYER;

/*			if (marker_areas[m].inside(mx, my) or marker_label_areas[m].inside(mx, my)) {
				s.marker = m;
				s.type = HoverData::Type::MARKER;
				return s;
			}
		}
	}*/
	return s;
}

