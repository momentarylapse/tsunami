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
namespace scenegraph {
	class SceneGraph;
	class Node;
}
class CpuDisplay;
class PeakMeterDisplay;
class Dial;
class BottomBarExpandButton;
class LogNotifier;

class MainView : public obs::Node<VirtualBase> {
public:
	MainView(Session *session, const string &id);

	string id;
	//int perf_channel;
	//vec2 m;
	owned<scenegraph::SceneGraph> scene_graph;
	CpuDisplay *cpu_display;
	PeakMeterDisplay *peak_meter_display;
	Dial *output_volume_dial;
	BottomBarExpandButton *bottom_bar_expand_button;
	LogNotifier *log_notifier;
};


#endif //TSUNAMI_MAINVIEW_H
