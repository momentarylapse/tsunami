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

class AudioView;
namespace scenegraph {
	class SceneGraph;
}
class Painter;

class MouseDelayAction {
public:
	MouseDelayAction() {}
	virtual ~MouseDelayAction() {}
	virtual void on_start(float mx, float my) {}
	virtual void on_update(float mx, float my) {}
	virtual void on_finish(float mx, float my) {}
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
	scenegraph::SceneGraph *scene_graph;
	typedef std::function<void()> Callback;
	owned<MouseDelayAction> action;
	void prepare(MouseDelayAction *action);
	void start_acting(float mx, float my);
	bool update(float mx, float my);
	bool has_focus();
	bool acting();
	void finish(float mx, float my);
	void cancel();
	void draw_post(Painter *p);
};

class SongSelection;
class AudioViewLayer;
enum class SelectionMode;

MouseDelayAction* CreateMouseDelayObjectsDnD(AudioViewLayer *l, const SongSelection &s);
MouseDelayAction* CreateMouseDelaySelect(AudioView *v, SelectionMode mode, bool override_start);

#endif /* SRC_VIEW_MOUSEDELAYPLANNER_H_ */
