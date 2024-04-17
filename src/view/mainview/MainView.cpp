//
// Created by michi on 4/17/24.
//

#include "MainView.h"
#include "LogNotifier.h"
#include "BottomBarExpandButton.h"
#include "../helper/graph/SceneGraph.h"
#include "../helper/PeakMeterDisplay.h"
#include "../helper/Dial.h"
#include "../helper/CpuDisplay.h"
#include "../bottombar/BottomBar.h"
#include "../TsunamiWindow.h"
#include "../MouseDelayPlanner.h"
#include "../ColorScheme.h"
#include "../../device/stream/AudioOutput.h"
#include "../../Playback.h"
#include "../../stuff/PerformanceMonitor.h"
#include "../../Session.h"



MainView::MainView(Session *_session, const string &_id) {
	session = _session;
	id = _id;
	auto win = session->win.get();
	//perf_channel = PerformanceMonitor::create_channel("view", this);

	themes.add(ColorSchemeBright());
	themes.add(ColorSchemeDark());
	themes.add(ColorSchemeSystem(win, id));


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

	bottom_bar_expand_button = new BottomBarExpandButton(session);
	scene_graph->add_child(bottom_bar_expand_button);

	log_notifier = new LogNotifier(session);
	scene_graph->add_child(log_notifier);

	cpu_display = new CpuDisplay(session);
	scene_graph->add_child(cpu_display);

	// TODO move "OnScreenDisplay"?
	peak_meter_display = new PeakMeterDisplay(session->playback->peak_meter.get(), PeakMeterDisplay::Mode::BOTH);
	peak_meter_display->align.dx = 90;
	peak_meter_display->align.dy = -20;
	peak_meter_display->align.horizontal = scenegraph::Node::AlignData::Mode::LEFT;
	peak_meter_display->align.vertical = scenegraph::Node::AlignData::Mode::BOTTOM;
	peak_meter_display->align.dz = 100;
	peak_meter_display->hidden = true;
	scene_graph->add_child(peak_meter_display);

	output_volume_dial = new Dial(_("output"), 0, 100);
	output_volume_dial->align.horizontal = scenegraph::Node::AlignData::Mode::LEFT;
	output_volume_dial->align.vertical = scenegraph::Node::AlignData::Mode::BOTTOM;
	output_volume_dial->align.dx = 230;
	output_volume_dial->align.dy = 0;
	output_volume_dial->align.dz = 100;
	//output_volume_dial->reference_value = 50;
	output_volume_dial->unit = "%";
	output_volume_dial->set_value(session->playback->output_stream->get_volume() * 100);
	output_volume_dial->out_value >> create_data_sink<float>([this] (float v) {
		session->playback->output_stream->set_volume(v / 100.0f);
	});
	session->playback->output_stream->out_changed >> create_sink([this] {
		output_volume_dial->set_value(session->playback->output_stream->get_volume() * 100);
	});
	output_volume_dial->hidden = true;
	scene_graph->add_child(output_volume_dial);

	onscreen_display = nullptr; //new scenegraph::NodeFree();


	set_theme(hui::config.get_str("View.ColorScheme", "system"));

	hui::run_later(1.0f, [this] {
		session->status("test");
		session->e("test");
	});

	win->activate(id);
}

void MainView::set_theme(const string &name) {
	hui::config.set_str("View.ColorScheme", name);
	theme = themes[0];
	for (auto &b: themes)
		if (b.name == name)
			theme = b;

	scene_graph->request_redraw();
}

void MainView::update_onscreen_displays() {
	// TODO move to MainView
	bottom_bar_expand_button->hidden = false;
	peak_meter_display->hidden = true;
	output_volume_dial->hidden = true;

	if (session->win->bottom_bar)
		if (!session->win->bottom_bar->is_active(BottomBar::MIXING_CONSOLE)) {
			peak_meter_display->hidden = !session->playback->is_active();
			output_volume_dial->hidden = !session->playback->is_active();
		}
	scene_graph->request_redraw();
}

