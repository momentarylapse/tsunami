/*
 * SceneGraph.h
 *
 *  Created on: 08.06.2019
 *      Author: michi
 */

#ifndef SRC_VIEW_NODE_SCENEGRAPH_H_
#define SRC_VIEW_NODE_SCENEGRAPH_H_

#include "ViewNode.h"

class SceneGraph : public ViewNode {
public:
	SceneGraph(AudioView *view);

	bool on_left_button_down() override;
	bool on_left_button_up() override;
	bool on_left_double_click() override;
	bool on_right_button_down() override;
	bool on_mouse_move() override;

	//ViewNode *get_hover();
	HoverData get_hover_data() override;

	void draw(Painter *p) override;

	string get_tip() override;
};

#endif /* SRC_VIEW_NODE_SCENEGRAPH_H_ */
