/*
 * Background.h
 *
 *  Created on: 08.06.2019
 *      Author: michi
 */

#ifndef SRC_VIEW_GRAPH_BACKGROUND_H_
#define SRC_VIEW_GRAPH_BACKGROUND_H_

#include "../Helper/Graph/Node.h"

class AudioView;
class AudioViewLayer;

class Background : public scenegraph::NodeFree {
public:
	Background(AudioView *view);

	bool on_left_button_down(float mx, float my) override;
	bool on_right_button_down(float mx, float my) override;
	bool allow_handle_click_when_gaining_focus() const override { return false; }

	void draw_layer_separator(Painter *c, AudioViewLayer *l1, AudioViewLayer *l2);

	void on_draw(Painter *p) override;

	AudioView *view;
	HoverData get_hover_data(float mx, float my);
};

#endif /* SRC_VIEW_GRAPH_BACKGROUND_H_ */
