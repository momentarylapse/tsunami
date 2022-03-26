/*
 * ViewMarker.h
 *
 *  Created on: 04.07.2019
 *      Author: michi
 */
 
#include "ViewMarker.h"
#include "../../Helper/Graph/Node.h"
#include "../../HoverData.h"

ViewMarker::ViewMarker(AudioViewTrack *parent, TrackMarker *_marker) {
	marker = _marker;
	set_perf_name("marker");
}


HoverData ViewMarker::get_hover_data(const vec2 &m) {
	auto h = Node::get_hover_data(m);
	//h.vlayer = this;
	h.marker = marker;
	//h.vtrack = view->get_track(layer->track);
	h.type = HoverData::Type::MARKER;

/*			if (marker_areas[m].inside(mx, my) or marker_label_areas[m].inside(mx, my)) {
				s.marker = m;
				s.type = HoverData::Type::MARKER;
				return h;
			}
		}
	}*/
	return h;
}

void ViewMarker::on_draw(Painter *p) {
}

bool ViewMarker::on_left_button_down(const vec2 &m) {
	return true;
}

bool ViewMarker::on_left_double_click(const vec2 &m) {
	return true;
}

bool ViewMarker::on_right_button_down(const vec2 &m) {
	return true;
}

