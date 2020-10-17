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
	
	bool playable();
	color color_bg();
	color color_frame();
	color color_text();

	bool on_left_button_down() override;
	bool on_left_double_click() override;
	bool on_right_button_down() override;
	
	void update_geometry_recursive(const rect &target_area) override;
};



#endif /* SRC_VIEW_GRAPH_TRACKHEADER_H_ */