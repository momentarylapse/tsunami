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

namespace hui {
	class Panel;
}

namespace scenegraph {

class SceneGraph : public Node {
public:
	using Callback = std::function<void()>;
	SceneGraph();
	void set_callback_set_current(Callback f);
	void set_callback_redraw(Callback f);

	bool on_left_button_down(float mx, float my) override;
	bool on_left_button_up(float mx, float my) override;
	bool on_left_double_click(float mx, float my) override;
	bool on_right_button_down(float mx, float my) override;
	bool on_right_button_up(float mx, float my) override;
	bool on_mouse_move(float mx, float my) override;
	bool on_mouse_wheel(float dx, float dy) override;
	bool on_key(int key) override;
	bool allow_handle_click_when_gaining_focus() override;

	//ViewNode *get_hover();
	HoverData get_hover_data(float mx, float my) override;

	void draw(Painter *p);
	void on_draw(Painter *p) override;

	string get_tip() override;

	HoverData hover;
	HoverData cur_selection;
	void set_current(const HoverData &h);
	Callback cb_set_current;
	Callback cb_redraw;

	owned<MouseDelayPlanner> mdp;
	void mdp_prepare(MouseDelayAction *action);
	void mdp_run(MouseDelayAction *action, float mx, float my);
	void mdp_prepare(Callback update);

	float mx, my;
	void set_mouse(float mx, float my);

	void integrate(hui::Panel *panel, const string &id, std::function<void(Painter *)> custom_draw, bool fill);
	static SceneGraph *create_integrated(hui::Panel *panel, const string &id, Node *node, const string &perf_name, std::function<void(Painter *)> custom_draw=nullptr, bool fill=true);
};

}

#endif /* SRC_VIEW_NODE_SCENEGRAPH_H_ */
