/*
 * CpuDisplay.h
 *
 *  Created on: 10.03.2018
 *      Author: michi
 */

#ifndef SRC_VIEW_HELPER_CPUDISPLAY_H_
#define SRC_VIEW_HELPER_CPUDISPLAY_H_

#include "../../lib/pattern/Observable.h"
#include "graph/Node.h"

namespace hui {
	class Panel;
	class Dialog;
}
namespace scenegraph {
	class SceneGraph;
}
class Painter;

namespace tsunami {

class PerformanceMonitor;
class Session;
class AudioView;
class PerfChannelInfo;

class CpuDisplay : public scenegraph::NodeFree {
public:
	CpuDisplay(Session *session);
	virtual ~CpuDisplay();

	bool on_left_button_down(const vec2 &m) override;
	bool on_mouse_wheel(const vec2 &d) override;
	void on_draw(Painter *p) override;

	void draw_background(Painter *p);
	void draw_graphs(Painter *p);
	void draw_table(Painter *p);

	void update();
	void enable(bool active);

	float table_height() const;

	Session *session;
	PerformanceMonitor *perf_mon;
	AudioView *view;
	bool large;
	obs::sink in_perf_mon_update;

	owned<hui::Dialog> dlg;

	Array<PerfChannelInfo> channels;
	Array<int> expanded;
	bool show_sleeping;
	bool show_total;
	float scroll_offset;

	float line_dy;
	float line_offset;
	float indent_dx;
};

}

#endif /* SRC_VIEW_HELPER_CPUDISPLAY_H_ */
