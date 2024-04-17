//
// Created by michi on 4/17/24.
//

#include "MainView.h"
#include "../helper/graph/SceneGraph.h"
#include "LogNotifier.h"
#include "../TsunamiWindow.h"
#include "../MouseDelayPlanner.h"
#include "../ColorScheme.h"
#include "../../stuff/PerformanceMonitor.h"
#include "../../Session.h"



MainView::MainView(Session *session, const string &_id) {
	id = _id;
	auto win = session->win.get();
	//perf_channel = PerformanceMonitor::create_channel("view", this);

	auto pad = new scenegraph::Node;

	scene_graph = scenegraph::SceneGraph::create_integrated(win, id, pad, "view", [this] (Painter *p) {
		p->set_font_size(theme.FONT_SIZE);
		scene_graph->update_geometry_recursive(p->area());
		//pad->_update_scrolling();
		scene_graph->draw(p);

		/*auto m = hui::get_event()->m;

		string tip;
		if (graph->hover.node)
			tip = graph->hover.node->get_tip();
		if (tip.num > 0) {
			p->set_font("", theme.FONT_SIZE, true, false);
			draw_cursor_hover(p, tip, m, graph->area);
			p->set_font("", theme.FONT_SIZE, false, false);
		}*/

		bool animating = false;

		if (log_notifier->progress(0.03f))
			animating = true;

		//if (animating)
		//	draw_runner_id = hui::run_later(animating ? 0.03f : 0.2f, [this]{ force_redraw(); });
	});

	//bottom_bar_expand_button = new BottomBarExpandButton(this);
	//scene_graph->add_child(bottom_bar_expand_button);

	log_notifier = new LogNotifier(session);
	scene_graph->add_child(log_notifier);

	hui::run_later(1.0f, [session, this] {
		session->status("test");
		session->e("test");
	});

	win->activate(id);
}

