/*
 * ControlColorButton.h
 *
 *  Created on: 17.06.2013
 *      Author: michi
 */

#ifndef CONTROLCOLORBUTTON_H_
#define CONTROLCOLORBUTTON_H_

#include "Control.h"
#include "../../image/color.h"

#include <gtk/gtk.h>

namespace hui
{

class ControlColorButton : public Control
{
public:
	ControlColorButton(const string &text, const string &id);

	void __set_option(const string &op, const string &value) override;
	void __set_color(const color &c) override;
	color get_color() override;

private:
	color _last_set;
	GdkRGBA _last_set_gdk;
#if GTK_CHECK_VERSION(4,10,0)
	GtkColorDialog *dialog;
#endif
};

};

#endif /* CONTROLCOLORBUTTON_H_ */
