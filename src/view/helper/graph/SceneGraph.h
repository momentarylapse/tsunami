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

namespace hui {
	class Panel;
}

namespace scenegraph {

class MouseDelayPlanner;
class MouseDelayAction;

class SceneGraph : public Node {
public:
	using MouseCallback = std::function<void(const vec2&)>;
	SceneGraph();

	obs::source out_current_changed{this, "current-changed"};
	obs::source out_redraw{this, "redraw"};

	bool on_left_button_down(const vec2 &m) override;
	bool on_left_button_up(const vec2 &m) override;
	bool on_left_double_click(const vec2 &m) override;
	bool on_right_button_down(const vec2 &m) override;
	bool on_right_button_up(const vec2 &m) override;
	bool on_mouse_move(const vec2 &m) override;
	bool on_mouse_wheel(const vec2 &d) override;
	bool on_key_down(int key) override;
	bool on_key_up(int key) override;
	bool on_gesture(const string &id, const vec2 &m, const vec2 &param) override;
	bool allow_handle_click_when_gaining_focus() const override;

	//ViewNode *get_hover();
	tsunami::HoverData get_hover_data(const vec2 &m) override;

	void draw(Painter *p);
	void on_draw(Painter *p) override;

	string get_tip() const override;

	tsunami::HoverData hover;
	tsunami::HoverData cur_selection;
	void set_current(const tsunami::HoverData &h);

	owned<MouseDelayPlanner> mdp;
	void mdp_prepare(MouseDelayAction *action, const vec2 &m);
	void mdp_run(MouseDelayAction *action, const vec2 &m);
	void mdp_prepare(MouseCallback update, const vec2 &m);

	vec2 __m;
	void set_mouse(const vec2 &m);
	void update_hover(const vec2& m);

	hui::Panel* panel = nullptr;

	void integrate(hui::Panel *panel, const string &id, std::function<void(Painter *)> custom_draw, bool fill);
	static SceneGraph *create_integrated(hui::Panel *panel, const string &id, Node *node, const string &perf_name, std::function<void(Painter *)> custom_draw=nullptr, bool fill=true);
};

}

#endif /* SRC_VIEW_NODE_SCENEGRAPH_H_ */
