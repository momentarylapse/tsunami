/*
 * ColorScheme.cpp
 *
 *  Created on: 21.06.2015
 *      Author: michi
 */

#include "ColorScheme.h"
#include <math.h>



ColorScheme ColorSchemeBasic::create(bool active) const
{
	ColorScheme c;
	c.name = name;
	c.background = background;

	if (active){
		c.text = text;
		c.selection = selection;
		c.hover = hover;
	}else{
		c.text = ColorInterpolate(text, background, 0.3f);
		c.selection = ColorInterpolate(selection, background, pow(0.3f, gamma));
		c.hover = ColorInterpolate(hover, background, 0.3f);
	}

	//c.background_track_selected = ColorInterpolate(background, c.selection*1.5f, 0.17f);
	//c.background_track = ColorInterpolate(c.background, c.background_track_selected, 0.5f);
	c.background_track_selected = ColorInterpolate(background, c.selection*1.2f, 0.15f);
	c.background_track = c.background;
	c.selection_internal = c.selection;
	c.selection_internal.a = pow(0.2f, gamma);
	c.selection_boundary = c.selection;
	c.selection_boundary_hover = ColorInterpolate(c.selection, c.hover, 0.6f);
	c.preview_marker = color(1, 0, 0.7f, 0);
	c.capture_marker = color(1, 0.7f, 0, 0);
	c.text_soft1 = ColorInterpolate(background, c.text, pow(0.72f, gamma));
	c.text_soft3 = ColorInterpolate(background, ColorInterpolate(c.text, c.selection, 0.7f), pow(0.3f, gamma));
	c.text_soft2 = ColorInterpolate(c.text_soft3, c.text_soft1, 0.4f);
	c.grid = c.text_soft3;
	c.sample = c.text_soft2;
	c.sample_selected = c.selection;
	return c;
}

