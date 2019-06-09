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

class MouseDelayPlanner {
public:
	MouseDelayPlanner(AudioView *view);

	float dist = -1;
	int pos0 = 0;
	float x0 = 0, y0 = 0;
	int min_move_to_start;
	AudioView *view;
	typedef std::function<void()> Callback;
	Callback cb_start, cb_update, cb_end;
	void prepare(Callback cb_start, Callback cb_update, Callback cb_end);
	bool update();
	bool has_focus();
	bool acting();
	void stop();
};

#endif /* SRC_VIEW_MOUSEDELAYPLANNER_H_ */
