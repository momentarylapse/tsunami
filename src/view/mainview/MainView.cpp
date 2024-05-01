//
// Created by michi on 4/17/24.
//

#include "MainView.h"
#include "LogNotifier.h"
#include "BottomBarExpandButton.h"
#include "../helper/graph/SceneGraph.h"
#include "../helper/graph/ToolTipOverlay.h"
#include "../helper/PeakMeterDisplay.h"
#include "../helper/Dial.h"
#include "../helper/CpuDisplay.h"
#include "../audioview/AudioView.h"
#include "../bottombar/BottomBar.h"
#include "../bottombar/signaleditor/SignalEditorTab.h"
#include "../TsunamiWindow.h"
#include "../MouseDelayPlanner.h"
#include "../ColorScheme.h"
#include "../../device/stream/AudioOutput.h"
#include "../../stuff/PerformanceMonitor.h"
#include "../../module/SignalChain.h"
#include "../../Playback.h"
#include "../../Session.h"

class TabBarButton : public scenegraph::Node {
public:
	TabBarButton(MainView *_main_view, scenegraph::Node* _view) {
		align.w = 200;
		align.horizontal = AlignData::Mode::LEFT;
		align.vertical = AlignData::Mode::FILL;
		set_perf_name("button");
		main_view = _main_view;
		view = _view;
	}
	string title() const {
		if (auto av = dynamic_cast<AudioView*>(view)) {
			return "song..."; //av->song->
		}
		if (auto e = dynamic_cast<SignalEditorTab*>(view)) {
			return "chain: " + e->chain->name;
		}
		return PerformanceMonitor::get_name(view->perf_channel);
	}
	void on_draw(Painter* p) override {
		p->set_color(theme.text_soft2);
		if (is_cur_hover())
			p->set_color(theme.text);
		if (view == main_view->active_view)
			p->set_font("", theme.FONT_SIZE_SMALL, true, false);
		else
			p->set_font("", theme.FONT_SIZE_SMALL, false, false);
		p->draw_str({area.x1, area.y1}, title());
	}
	bool on_left_button_down(const vec2& m) override {
		main_view->activate_view(view);
		return true;
	}
	MainView *main_view;
	scenegraph::Node* view;
};

class TabBar : public scenegraph::HBox {
public:
	TabBar(MainView *_main_view) {
		align.h = 25;
		align.horizontal = AlignData::Mode::FILL;
		align.vertical = AlignData::Mode::TOP;
		set_perf_name("tabbar");
		main_view = _main_view;
	}
	void on_draw(Painter* p) override {
		p->set_color(theme.background_track);
		p->draw_rect(area);
	}
	MainView *main_view;
	void rebuild() {
		children.clear();
		auto dummy = new Node;
		dummy->align.w = 80;
		dummy->align.horizontal = AlignData::Mode::LEFT;
		dummy->align.vertical = AlignData::Mode::FILL;
		add_child(dummy);
		for (auto v: weak(main_view->views))
			add_child(new TabBarButton(main_view, v));
	}
};


MainView::MainView(Session *_session, const string &_id) {
	session = _session;
	id = _id;
	auto win = session->win.get();
	//perf_channel = PerformanceMonitor::create_channel("view", this);

	themes.add(ColorSchemeBright());
	themes.add(ColorSchemeDark());
	themes.add(ColorSchemeSystem(win, id));


	vbox = new scenegraph::VBox;
	tab_bar = new TabBar(this);
	//tab_bar->set_hidden(true);
	vbox->add_child(tab_bar.get());

	scene_graph = scenegraph::SceneGraph::create_integrated(win, id, vbox.get(), "view", [this] (Painter *p) {
		p->set_font_size(theme.FONT_SIZE);
		scene_graph->update_geometry_recursive(p->area());
		scene_graph->draw(p);

		bool animating = false;

		if (log_notifier->progress(0.03f))
			animating = true;

		if (animating)
			draw_runner_id = hui::run_later(animating ? 0.03f : 0.2f, [this]{
				scene_graph->request_redraw();
			});
	});

	bottom_bar_expand_button = new BottomBarExpandButton(session);
	scene_graph->add_child(bottom_bar_expand_button);

	log_notifier = new LogNotifier(session);
	scene_graph->add_child(log_notifier);

	cpu_display = new CpuDisplay(session);
	scene_graph->add_child(cpu_display);

	scene_graph->add_child(new ToolTipOverlay);

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

	win->activate(id);
}

MainView::~MainView() {
	if (draw_runner_id >= 0)
		hui::cancel_runner(draw_runner_id);
}


void MainView::add_view(scenegraph::Node* view) {
	views.add(view);
	vbox->children.clear();
	for (auto v: weak(views))
		vbox->add_child(v);
	vbox->add_child(tab_bar.get());

	activate_view(view);
}

void MainView::activate_view(scenegraph::Node* view) {
	for (auto v: weak(views))
		v->set_hidden(v != view);
	tab_bar->set_hidden(views.num < 2);
	tab_bar->rebuild();
	active_view = view;
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

