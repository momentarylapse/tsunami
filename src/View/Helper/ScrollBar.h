/*
 * ScrollBar.h
 *
 *  Created on: 22.08.2018
 *      Author: michi
 */

#ifndef SRC_VIEW_HELPER_SCROLLBAR_H_
#define SRC_VIEW_HELPER_SCROLLBAR_H_

#include "../../lib/base/base.h"
#include "../../lib/math/rect.h"

class Painter;

class ScrollBar
{
public:
	float offset = 0;
	float page_size = 0;
	float content_size = 0;
	rect area = r_id;
	float mouse_offset = 0;
	void drag_start(float mx, float my);
	void drag_update(float mx, float my);
	void draw(Painter *c, bool hover);
	void set_area(const rect &r);
	void update(float page, float content);
};

#endif /* SRC_VIEW_HELPER_SCROLLBAR_H_ */
