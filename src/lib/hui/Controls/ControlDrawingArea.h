/*
 * ControlDrawingArea.h
 *
 *  Created on: 17.06.2013
 *      Author: michi
 */

#ifndef CONTROLDRAWINGAREA_H_
#define CONTROLDRAWINGAREA_H_

#include "Control.h"
#include <mutex>

#include <gtk/gtk.h>

struct rect;

namespace hui
{

class Timer;

class ControlDrawingArea : public Control {
public:
	ControlDrawingArea(const string &text, const string &id);
	~ControlDrawingArea();
	void make_current();

#if GTK_CHECK_VERSION(4,0,0)
	void disable_event_handlers_rec() override;
	void __gtk_add_controller(GtkEventController* controller);
	Array<GtkEventController*> __controllers;
#endif

	void __set_option(const string &op, const string &value) override;

	void *cur_cairo;
	bool is_opengl;

	//Array<rect> redraw_area;
	//Timer *delay_timer;

	//std::mutex mutex;

	void redraw();
	void redraw_partial(const rect &r);
};

};

#endif /* CONTROLDRAWINGAREA_H_ */
