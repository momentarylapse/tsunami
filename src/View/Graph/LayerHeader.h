/*
 * LayerHeader.h
 *
 *  Created on: 08.06.2019
 *      Author: michi
 */

#ifndef SRC_VIEW_GRAPH_LAYERHEADER_H_
#define SRC_VIEW_GRAPH_LAYERHEADER_H_

#include "../Helper/Graph/Node.h"

class AudioViewLayer;


class LayerHeader : public scenegraph::NodeRel {
public:
	AudioViewLayer *vlayer;
	LayerHeader(AudioViewLayer *l);
	void on_draw(Painter *c) override;

	HoverData get_hover_data(float mx, float my) override;

	bool playable();
	color color_bg();
	color color_frame();
	color color_text();

	bool on_left_button_down(float mx, float my) override;
	bool on_right_button_down(float mx, float my) override;
	void update_geometry_recursive(const rect &target_area) override;
};



#endif /* SRC_VIEW_GRAPH_LAYERHEADER_H_ */
