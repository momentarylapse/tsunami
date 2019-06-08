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

	void on_left_button_down() override;
	void on_left_button_up() override;
	void on_right_button_down() override;
	void on_mouse_move() override;

	Selection get_hover() override;
	void draw(Painter *p) override;

	string get_tip() override;
};

#endif /* SRC_VIEW_NODE_SCENEGRAPH_H_ */
