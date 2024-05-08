//
// Created by michi on 4/17/24.
//

#ifndef TSUNAMI_MAINVIEW_H
#define TSUNAMI_MAINVIEW_H

#include "base/pointer.h"
#include "pattern/Observable.h"
#include "math/vec2.h"


class Session;
class rect;
class Painter;
class ColorScheme;
namespace scenegraph {
	class SceneGraph;
	class Node;
}
class CpuDisplay;
class PeakMeterDisplay;
class Dial;
class BottomBarExpandButton;
class LogNotifier;
class TabBar;
class MainViewNode;

class MainView : public obs::Node<VirtualBase> {
public:
	MainView(Session *session, const string &id);
	~MainView();

	obs::xsource<MainViewNode*> out_view_changed{this, "view-changed"};

	void update_onscreen_displays();

	string id;
	Session* session;
	owned<scenegraph::SceneGraph> scene_graph;
	int draw_runner_id = -1;
	shared<scenegraph::Node> vbox;
	shared<TabBar> tab_bar;
	CpuDisplay *cpu_display;
	PeakMeterDisplay *peak_meter_display;
	Dial *output_volume_dial;
	BottomBarExpandButton *bottom_bar_expand_button;
	LogNotifier *log_notifier;
	scenegraph::Node *onscreen_display;

	shared_array<MainViewNode> views;
	MainViewNode* active_view = nullptr;

	void _add_view(MainViewNode* view);
	void _activate_view(MainViewNode* view);
	void open_for(VirtualBase* p);


	Array<ColorScheme> themes;
	void set_theme(const string &name);
};


#endif //TSUNAMI_MAINVIEW_H
