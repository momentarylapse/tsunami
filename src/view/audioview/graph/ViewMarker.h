/*
 * ViewMarker.h
 *
 *  Created on: 04.07.2019
 *      Author: michi
 */

#ifndef SRC_VIEW_GRAPH_VIEWMARKER_H_
#define SRC_VIEW_GRAPH_VIEWMARKER_H_

#include "../../helper/graph/Node.h"

namespace tsunami {

class TrackMarker;
class AudioViewTrack;

class ViewMarker : public ::scenegraph::Node {
public:
	ViewMarker(AudioViewTrack *parent, TrackMarker *marker);

	void on_draw(Painter *c) override;
	HoverData get_hover_data(const vec2 &m) override;

	bool on_left_button_down(const vec2 &m) override;
	bool on_left_double_click(const vec2 &m) override;
	bool on_right_button_down(const vec2 &m) override;

	TrackMarker *marker;
};

}

#endif // SRC_VIEW_GRAPH_VIEWMARKER_H_
