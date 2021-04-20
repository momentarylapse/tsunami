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
	using Callback = std::function<void()>;
	CpuDisplay(Session *session, Callback request_redraw);
	virtual ~CpuDisplay();

	bool on_left_button_down(float mx, float my) override;
	bool on_mouse_wheel(float dx, float dy) override;
	void on_draw(Painter *p) override;

	void draw_background(Painter *p);
	void draw_graphs(Painter *p);
	void draw_table(Painter *p);

	void update();
	void enable(bool active);

	Session *session;
	PerformanceMonitor *perf_mon;
	AudioView *view;
	bool large;
	rect area_graph;

	Callback request_redraw;

	hui::Dialog *dlg;


	Array<PerfChannelInfo> channels;
	Array<int> expanded;
	bool show_sleeping;
	bool show_total;
	float scroll_offset;
};


#endif /* SRC_VIEW_HELPER_CPUDISPLAY_H_ */
