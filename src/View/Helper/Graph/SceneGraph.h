/*
 * SceneGraph.h
 *
 *  Created on: 08.06.2019
 *      Author: michi
 */

#ifndef SRC_VIEW_HELPER_GRAPH_SCENEGRAPH_H_
#define SRC_VIEW_HELPER_GRAPH_SCENEGRAPH_H_

#include "Node.h"
#include "../../HoverData.h"

class AudioView;
class MouseDelayPlanner;
class MouseDelayAction;

namespace scenegraph {

class SceneGraph : public Node {
public:
	using Callback = std::function<void()>;
	SceneGraph(Callback cb_set_current);

	bool on_left_button_down(float mx, float my) override;
	bool on_left_button_up(float mx, float my) override;
	bool on_left_double_click(float mx, float my) override;
	bool on_right_button_down(float mx, float my) override;
	bool on_mouse_move(float mx, float my) override;
	bool allow_handle_click_when_gaining_focus() override;

	//ViewNode *get_hover();
	HoverData get_hover_data(float mx, float my) override;

	void on_draw(Painter *p) override;

	string get_tip() override;

	HoverData hover;
	HoverData cur_selection;
	void set_current(const HoverData &h);
	Callback cb_set_current;

	owned<MouseDelayPlanner> mdp;
	void mdp_prepare(MouseDelayAction *action);
	void mdp_run(MouseDelayAction *action, float mx, float my);
	void mdp_prepare(Callback update);

	float mx, my;
	void set_mouse(float mx, float my);
};

}

#endif /* SRC_VIEW_NODE_SCENEGRAPH_H_ */
