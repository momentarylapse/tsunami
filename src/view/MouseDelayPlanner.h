/*
 * MouseDelayPlanner.h
 *
 *  Created on: 09.06.2019
 *      Author: michi
 */

#ifndef SRC_VIEW_MOUSEDELAYPLANNER_H_
#define SRC_VIEW_MOUSEDELAYPLANNER_H_

#include "../lib/base/pointer.h"
#include <functional>

class Painter;
class vec2;

namespace scenegraph {
	class SceneGraph;

class MouseDelayAction {
public:
	MouseDelayAction() {}
	virtual ~MouseDelayAction() {}
	virtual void on_start(const vec2 &m) {}
	virtual void on_update(const vec2 &m) {}
	virtual void on_finish(const vec2 &m) {}
	virtual void on_cancel() {}
	virtual void on_clean_up() {}
	virtual void on_draw_post(Painter *p) {}
	scenegraph::SceneGraph *scene_graph = nullptr;
};

class MouseDelayPlanner {
public:
	MouseDelayPlanner(scenegraph::SceneGraph *scene_graph);

	float dist = -1;
	float x0 = 0, y0 = 0;
	int min_move_to_start;
	bool _started_acting = false;
	::scenegraph::SceneGraph *scene_graph;
	typedef std::function<void()> Callback;
	owned<MouseDelayAction> action;
	void prepare(MouseDelayAction *action);
	void start_acting(const vec2 &m);
	bool update(const vec2 &m);
	bool has_focus();
	bool acting();
	void finish(const vec2 &m);
	void cancel();
	void draw_post(Painter *p);
};

}

namespace tsunami {

class AudioView;
class SongSelection;
class AudioViewLayer;
enum class SelectionMode;

scenegraph::MouseDelayAction* CreateMouseDelayObjectsDnD(AudioViewLayer *l, const SongSelection &s);
scenegraph::MouseDelayAction* CreateMouseDelaySelect(AudioView *v, SelectionMode mode, bool override_start);

}

#endif /* SRC_VIEW_MOUSEDELAYPLANNER_H_ */
