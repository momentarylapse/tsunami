/*
 * CpuDisplay.h
 *
 *  Created on: 10.03.2018
 *      Author: michi
 */

#ifndef SRC_VIEW_HELPER_CPUDISPLAY_H_
#define SRC_VIEW_HELPER_CPUDISPLAY_H_

#include "../../Stuff/Observable.h"
#include "Graph/Node.h"

namespace hui {
	class Panel;
	class Dialog;
}
namespace scenegraph {
	class SceneGraph;
}
class Painter;
class PerformanceMonitor;
class Session;
class AudioView;
class PerfChannelInfo;

class CpuDisplay : public scenegraph::NodeFree {
public:
	CpuDisplay(Session *session, hui::Callback request_redraw);
	virtual ~CpuDisplay();

	bool on_left_button_down() override;
	void draw(Painter *p) override;
	void update();
	void enable(bool active);

	Session *session;
	PerformanceMonitor *perf_mon;
	AudioView *view;

	hui::Callback request_redraw;

	hui::Dialog *dlg;


	Array<PerfChannelInfo> channels;
};



class CpuDisplayAdapter : public VirtualBase {
public:
	CpuDisplayAdapter(hui::Panel *parent, const string &id, CpuDisplay *cpu_display);

	CpuDisplay *cpu_display;
	scenegraph::SceneGraph *scene_graph;

	hui::Panel *parent;
	string id;
};

#endif /* SRC_VIEW_HELPER_CPUDISPLAY_H_ */
