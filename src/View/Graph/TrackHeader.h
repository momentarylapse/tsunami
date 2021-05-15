/*
 * TrackHeader.h
 *
 *  Created on: 08.06.2019
 *      Author: michi
 */

#ifndef SRC_VIEW_GRAPH_TRACKHEADER_H_
#define SRC_VIEW_GRAPH_TRACKHEADER_H_

#include "../Helper/Graph/Node.h"

class AudioViewTrack;
class AudioView;


class TrackHeader : public scenegraph::NodeRel {
public:
	AudioView *view;
	AudioViewTrack *vtrack;
	TrackHeader(AudioViewTrack *t);
	void on_draw(Painter *c) override;
	HoverData get_hover_data(float mx, float my) override;
	
	bool playable() const;
	color color_bg() const;
	color color_frame() const;
	color color_text() const;
	string nice_title() const;

	bool on_left_button_down(float mx, float my) override;
	bool on_left_double_click(float mx, float my) override;
	bool on_right_button_down(float mx, float my) override;
	
	void update_geometry_recursive(const rect &target_area) override;
};



#endif /* SRC_VIEW_GRAPH_TRACKHEADER_H_ */
