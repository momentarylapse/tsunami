/*
 * TrackHeader.h
 *
 *  Created on: 08.06.2019
 *      Author: michi
 */

#ifndef SRC_VIEW_GRAPH_TRACKHEADER_H_
#define SRC_VIEW_GRAPH_TRACKHEADER_H_

#include "../../helper/graph/Node.h"

namespace tsunami {

class AudioViewTrack;
class AudioView;


class TrackHeader : public ::scenegraph::NodeRel {
public:
	AudioView *view;
	AudioViewTrack *vtrack;

	explicit TrackHeader(AudioViewTrack *t);
	void on_draw(Painter *c) override;
	HoverData get_hover_data(const vec2 &m) override;
	
	bool playable() const;
	color color_bg(bool allow_hover = true) const;
	color color_text() const;
	string nice_title() const;

	bool on_left_button_down(const vec2 &m) override;
	bool on_left_double_click(const vec2 &m) override;
	bool on_right_button_down(const vec2 &m) override;
	
	void update_geometry_recursive(const rect &target_area) override;
};

}

#endif /* SRC_VIEW_GRAPH_TRACKHEADER_H_ */
