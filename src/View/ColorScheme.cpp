/*
 * ColorScheme.cpp
 *
 *  Created on: 21.06.2015
 *      Author: michi
 */

#include "ColorScheme.h"
#include <math.h>

color col_inter(const color a, const color &b, float t)
{
	float e = 0.7f;
	color c;
	c.a = 1;//(1-t) * a.a + t * b.a;
	c.r = pow((1-t) * pow(a.r, e) + t * pow(b.r, e), 1/e);
	c.g = pow((1-t) * pow(a.g, e) + t * pow(b.g, e), 1/e);
	c.b = pow((1-t) * pow(a.b, e) + t * pow(b.b, e), 1/e);
	return c;
}


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

	if (text.r > background.r){
		c.high_contrast_a = White;
		c.high_contrast_b = Black;
	}else{
		c.high_contrast_a = Black;
		c.high_contrast_b = White;
	}

	//c.background_track_selected = ColorInterpolate(background, c.selection*1.5f, 0.17f);
	//c.background_track = ColorInterpolate(c.background, c.background_track_selected, 0.5f);
	c.background_track_selected = col_inter(background, c.selection*1.2f, 0.15f);
	c.background_track_selection = col_inter(background, c.selection*1.2f, 0.4f);
	c.background_track = c.background;
	c.selection_bars = c.selection;
	c.selection_bars.a = pow(0.2f, gamma);
	c.selection_bars_hover = ColorInterpolate(c.selection_bars, hover, 0.5f);
	c.selection_internal = c.selection;
	c.selection_internal.a = pow(0.1f, gamma);
	c.selection_boundary = c.selection;
	c.selection_boundary_hover = ColorInterpolate(c.selection, c.hover, 0.6f);
	c.preview_marker = color(1, 0, 0.7f, 0);
	c.capture_marker = color(1, 0.7f, 0, 0);
	/*c.text_soft1 = ColorInterpolate(background, c.text, pow(0.72f, gamma));
	c.text_soft3 = ColorInterpolate(background, ColorInterpolate(c.text, c.selection, 0.3f), pow(0.3f, gamma));
	c.text_soft2 = ColorInterpolate(c.text_soft3, c.text_soft1, 0.4f);
	c.grid = ColorInterpolate(background, ColorInterpolate(c.text, c.selection, 0.7f), pow(0.2f, gamma));
*/

	c.text_soft1 = col_inter(background, c.text, 0.72f);
	c.text_soft3 = col_inter(background, col_inter(c.text, c.selection, 0.3f), 0.3f);
	c.text_soft2 = col_inter(c.text_soft3, c.text_soft1, 0.4f);
	c.grid = col_inter(background, col_inter(c.text, c.selection, 0.4f), 0.35f);
	c.grid_selected = col_inter(c.grid, c.selection, 0.4f);

	//c.grid = c.text_soft3;
	c.sample = c.text_soft2;
	c.sample_selected = c.selection;

	c.red = ColorInterpolate(Red, background, 0.3f);
	c.green = ColorInterpolate(Green, background, 0.3f);
	c.blue = ColorInterpolate(Blue, background, 0.3f);
	c.white = ColorInterpolate(White, background, 0.3f);
	c.red_hover = ColorInterpolate(c.red, hover, 0.3f);
	c.green_hover = ColorInterpolate(c.green, hover, 0.3f);
	c.blue_hover = ColorInterpolate(c.blue, hover, 0.3f);
	c.white_hover = ColorInterpolate(c.white, hover, 0.3f);
	return c;
}

