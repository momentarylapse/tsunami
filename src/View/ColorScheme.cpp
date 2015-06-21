/*
 * ColorScheme.cpp
 *
 *  Created on: 21.06.2015
 *      Author: michi
 */

#include "ColorScheme.h"
#include <math.h>



void ColorScheme::create(ColorSchemeBasic &basic)
{
	name = basic.name;
	background = basic.background;
	background_track_selected = ColorInterpolate(basic.background, basic.selection*1.5f, 0.17f);
	background_track = ColorInterpolate(background, background_track_selected, 0.5f);
	selection = basic.selection;
	hover = basic.hover;
	selection_internal = basic.selection;
	selection_internal.a = 0.2f;
	selection_boundary = basic.selection;
	selection_boundary_hover = ColorInterpolate(basic.selection, basic.hover, 0.6f);
	preview_marker = color(1, 0, 0.7f, 0);
	capture_marker = color(1, 0.7f, 0, 0);
	text = basic.text;
	text_soft1 = ColorInterpolate(basic.background, basic.text, pow(0.72f, basic.gamma));
	text_soft3 = ColorInterpolate(basic.background, ColorInterpolate(basic.text, basic.selection, 0.7f), pow(0.3f, basic.gamma));
	text_soft2 = ColorInterpolate(text_soft3, text_soft1, 0.4f);
	grid = text_soft3;
	sample = color(1, 0.6f, 0.6f, 0);
	sample_hover = color(1, 0.6f, 0, 0);
	sample_selected = color(1, 0.4f, 0.4f, 0.4f);
}

