/*
 * MouseDelayPlanner.h
 *
 *  Created on: 09.06.2019
 *      Author: michi
 */

#ifndef SRC_VIEW_MOUSEDELAYPLANNER_H_
#define SRC_VIEW_MOUSEDELAYPLANNER_H_

#include <functional>

class AudioView;
class Painter;

class MouseDelayAction {
public:
	MouseDelayAction() {}
	virtual ~MouseDelayAction() {}
	virtual void on_start() {}
	virtual void on_update() {}
	virtual void on_end() {}
	virtual void on_draw_post(Painter *p) {}
};

class MouseDelayPlanner {
public:
	MouseDelayPlanner(AudioView *view);

	float dist = -1;
	int pos0 = 0;
	float x0 = 0, y0 = 0;
	int min_move_to_start;
	AudioView *view;
	typedef std::function<void()> Callback;
	MouseDelayAction *action = nullptr;
	void prepare(MouseDelayAction *action);
	bool update();
	bool has_focus();
	bool acting();
	void stop();
	void draw_post(Painter *p);
};

#endif /* SRC_VIEW_MOUSEDELAYPLANNER_H_ */
