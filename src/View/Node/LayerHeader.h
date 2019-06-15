/*
 * LayerHeader.h
 *
 *  Created on: 08.06.2019
 *      Author: michi
 */

#ifndef SRC_VIEW_NODE_LAYERHEADER_H_
#define SRC_VIEW_NODE_LAYERHEADER_H_

#include "ViewNode.h"

class AudioViewLayer;


class LayerHeader : public ViewNodeRel
{
public:
	AudioViewLayer *vlayer;
	LayerHeader(AudioViewLayer *l);
	void draw(Painter *c) override;

	HoverData get_hover_data(float mx, float my) override;

	bool on_left_button_down() override;
	bool on_right_button_down() override;
};



#endif /* SRC_VIEW_NODE_LAYERHEADER_H_ */
