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
	bool on_right_button_down() override;
	bool on_mouse_move() override;

	ViewNode *get_hover();

	void draw(Painter *p) override;

	string get_tip() override;

	ViewNode *hover_node = nullptr;
	//ViewNode *mouse_owner = nullptr;


	struct MouseDelayPlanner {
		float dist = -1;
		int pos0 = 0;
		float x0 = 0, y0 = 0;
		int min_move_to_start;
		AudioView *view;
		hui::Callback cb_start, cb_update, cb_end;
		void prepare(hui::Callback cb_start, hui::Callback cb_update, hui::Callback cb_end);
		void update();
		bool active();
		void stop();
	} mdp;
};

#endif /* SRC_VIEW_NODE_SCENEGRAPH_H_ */
