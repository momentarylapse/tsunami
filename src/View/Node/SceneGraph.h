/*
 * SceneGraph.h
 *
 *  Created on: 08.06.2019
 *      Author: michi
 */

#ifndef SRC_VIEW_NODE_SCENEGRAPH_H_
#define SRC_VIEW_NODE_SCENEGRAPH_H_

#include "ViewNode.h"
#include "../HoverData.h"

class AudioView;
class MouseDelayPlanner;
class MouseDelayAction;

class SceneGraph : public ViewNode {
public:
	SceneGraph(hui::Callback cb_set_current);//AudioView *view);

	bool on_left_button_down() override;
	bool on_left_button_up() override;
	bool on_left_double_click() override;
	bool on_right_button_down() override;
	bool on_mouse_move() override;
	bool allow_handle_click_when_gaining_focus() override;

	//ViewNode *get_hover();
	HoverData get_hover_data(float mx, float my) override;

	void draw(Painter *p) override;

	string get_tip() override;

	//AudioView *view;

	HoverData hover;
	HoverData cur_selection;
	void set_current(const HoverData &h);
	hui::Callback cb_set_current;

	owned<MouseDelayPlanner> mdp;
	void mdp_prepare(MouseDelayAction *action);
	void mdp_run(MouseDelayAction *action);
	void mdp_prepare(hui::Callback update);

	float mx, my;
	void set_mouse();
};

#endif /* SRC_VIEW_NODE_SCENEGRAPH_H_ */
