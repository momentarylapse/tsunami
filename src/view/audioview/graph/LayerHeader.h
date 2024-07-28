/*
 * LayerHeader.h
 *
 *  Created on: 08.06.2019
 *      Author: michi
 */

#ifndef SRC_VIEW_GRAPH_LAYERHEADER_H_
#define SRC_VIEW_GRAPH_LAYERHEADER_H_

#include "../../helper/graph/Node.h"

namespace tsunami {

class AudioViewLayer;

class LayerHeader : public ::scenegraph::NodeRel {
public:
	AudioViewLayer *vlayer;
	explicit LayerHeader(AudioViewLayer *l);
	void on_draw(Painter *c) override;

	HoverData get_hover_data(const vec2 &m) override;

	bool playable() const;
	color color_bg() const;
	color color_frame() const;
	color color_text() const;

	bool on_left_button_down(const vec2 &m) override;
	bool on_right_button_down(const vec2 &m) override;
	void update_geometry_recursive(const rect &target_area) override;
};

}

#endif /* SRC_VIEW_GRAPH_LAYERHEADER_H_ */
