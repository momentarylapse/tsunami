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
#include "../signaleditor/SignalEditorTab.h"
#include "../TsunamiWindow.h"
#include "../MouseDelayPlanner.h"
#include "../ColorScheme.h"
#include "../../module/stream/AudioOutput.h"
#include "../../data/Song.h"
#include "../../lib/hui/config.h"
#include "../../lib/hui/language.h"
#include "../../lib/image/Painter.h"
#include "../../module/SignalChain.h"
#include "../../Playback.h"
#include "../../Session.h"

namespace tsunami {

const static float TAB_BAR_HEIGHT = 35;

class TabBarButton : public scenegraph::Node {
public:
	TabBarButton(MainView *_main_view, MainViewNode* _view) {
		align.w = 200;
		align.horizontal = AlignData::Mode::Left;
		align.vertical = AlignData::Mode::Fill;
		set_perf_name("button");
		main_view = _main_view;
		view = _view;
	}
	void on_draw(Painter* p) override {
		color fg = theme.text_soft2;
		color bg = theme.background_overlay;
		if (is_cur_hover()) {
			fg = theme.text_soft1;
			bg = theme.hoverify(bg);
		}
		p->set_roundness(8);
		p->set_color(bg);
		p->set_fill(true);
		p->draw_rect(area.grow(-2));
		p->set_roundness(0);
		p->set_color(fg);
		if (view == main_view->active_view) {
			p->set_font("", theme.FONT_SIZE_SMALL, true, false);
			p->set_color(theme.text);
		} else {
			p->set_font("", theme.FONT_SIZE_SMALL, false, false);
		}
		p->draw_str({area.x1 + 8, area.center().y - theme.FONT_SIZE_SMALL * 0.5f}, view->mvn_description());
	}
	bool on_left_button_down(const vec2& m) override {
		main_view->activate_view(view);
		return true;
	}
	MainView *main_view;
	MainViewNode* view;
};

class TabBar : public scenegraph::HBox {
public:
	TabBar(MainView *_main_view) {
		align.h = TAB_BAR_HEIGHT;
		align.horizontal = AlignData::Mode::Fill;
		align.vertical = AlignData::Mode::Top;
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
		dummy->align.horizontal = AlignData::Mode::Left;
		dummy->align.vertical = AlignData::Mode::Fill;
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
	peak_meter_display = new PeakMeterDisplay(session->playback->peak_meter.get(), PeakMeterDisplay::Mode::Both);
	peak_meter_display->align.dx = 90;
	peak_meter_display->align.dy = -20;
	peak_meter_display->align.horizontal = scenegraph::Node::AlignData::Mode::Left;
	peak_meter_display->align.vertical = scenegraph::Node::AlignData::Mode::Bottom;
	peak_meter_display->align.dz = 100;
	peak_meter_display->hidden = true;
	scene_graph->add_child(peak_meter_display);

	output_volume_dial = new Dial(_("output"), 0, 100);
	output_volume_dial->align.horizontal = scenegraph::Node::AlignData::Mode::Left;
	output_volume_dial->align.vertical = scenegraph::Node::AlignData::Mode::Bottom;
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


void MainView::_update_box() {
	vbox->children.clear();
	for (auto v: weak(views))
		vbox->add_child(v);
	vbox->add_child(tab_bar.get());
}

void MainView::_add_view(shared<MainViewNode> view) {
	views.add(view);
	_update_box();

	if (views.num == 1)
		activate_view(view.get());

	view->out_delete_me >> create_sink([this, _view = view.get()] {
		_remove_view(_view);
	});
}

void MainView::_remove_view(MainViewNode* view) {
	for (int i=0; i<views.num; i++)
		if (views[i].get() == view) {
			// switch to another panel before deleting - keep active_view valid
			if (i >= 1)
				activate_view(views[0].get());
			views.erase(i);
			break;
		}

	_update_box();

	activate_view(active_view);
}

void MainView::activate_view(MainViewNode* view) {
	for (auto v: weak(views))
		v->set_hidden(v != view);
	tab_bar->set_hidden(views.num < 2);
	tab_bar->rebuild();
	if (view != active_view) {
		active_view = view;
		view->mvn_on_enter();
		out_view_changed(view);
	}
}

void MainView::open_for(VirtualBase* p) {
	for (auto v: weak(views))
		if (v->mvn_data() == p) {
			activate_view(v);
			return;
		}

	if (auto song = dynamic_cast<Song*>(p)) {
		_add_view(new AudioView(song->session));
		activate_view(weak(views).back());
	} else if (auto chain = dynamic_cast<SignalChain*>(p)) {
		_add_view(new SignalEditorTab(chain));
		activate_view(weak(views).back());
	}
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
		if (!session->win->bottom_bar->is_active(BottomBar::Index::MixingConsole)) {
			peak_meter_display->hidden = !session->playback->is_active();
			output_volume_dial->hidden = !session->playback->is_active();
		}
	scene_graph->request_redraw();
}

}

