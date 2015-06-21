/*
 * ColorScheme.h
 *
 *  Created on: 21.06.2015
 *      Author: michi
 */

#ifndef SRC_VIEW_COLORSCHEME_H_
#define SRC_VIEW_COLORSCHEME_H_

#include "../lib/base/base.h"
#include "../lib/image/color.h"

class ColorSchemeBasic
{
public:
	color background;
	color text;
	color selection;
	color hover;
	float gamma;
	string name;
};

class ColorScheme : public ColorSchemeBasic
{
public:
	color background_track;
	color background_track_selected;
	color selection_internal;
	color selection_boundary;
	color selection_boundary_hover;
	color preview_marker;
	color capture_marker;
	color text_soft1;
	color text_soft2;
	color text_soft3;
	color grid;
	color sample;
	color sample_selected;
	color sample_hover;
	void create(ColorSchemeBasic &basic);
};

#endif /* SRC_VIEW_COLORSCHEME_H_ */
