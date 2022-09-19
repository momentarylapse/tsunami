/*
 * ColorScheme.cpp
 *
 *  Created on: 21.06.2015
 *      Author: michi
 */

#include "ColorScheme.h"
#include "../lib/base/map.h"
#include <math.h>

namespace hui {
	class Panel;
	void get_style_colors(Panel *p, const string &id, base::map<string,color> &colors);
}


const float ColorScheme::FONT_SIZE = 10.0f;
const float ColorScheme::FONT_SIZE_SMALL = FONT_SIZE * 0.8f;
const int ColorScheme::MAX_TRACK_CHANNEL_HEIGHT = 74;
const float ColorScheme::LINE_WIDTH = 1.0f;
const float ColorScheme::CORNER_RADIUS = 8.0f;
const int ColorScheme::SAMPLE_FRAME_HEIGHT = 20;
const int ColorScheme::TIME_SCALE_HEIGHT = 20;
const int ColorScheme::TRACK_HANDLE_WIDTH = 120;
const int ColorScheme::LAYER_HANDLE_WIDTH = 60;
const int ColorScheme::TRACK_HANDLE_HEIGHT = TIME_SCALE_HEIGHT * 2;
const int ColorScheme::TRACK_HANDLE_HEIGHT_SMALL = TIME_SCALE_HEIGHT;
const float ColorScheme::SCROLLBAR_MINIMUM_HANDLE_SIZE = 15.0f;
const float ColorScheme::SCROLLBAR_D = 5;
const float ColorScheme::SCROLLBAR_WIDTH = 20;

ColorScheme theme;


const color PITCH_COLORS[12] = {
	color(1, 1.000000, 0.400000, 0.400000), // C
	color(1, 0.900000, 0.700000, 0.400000),
	color(1, 0.800000, 0.800000, 0.400000), // D
	color(1, 0.700000, 0.900000, 0.400000),
	color(1, 0.400000, 0.900000, 0.400000), // E
	color(1, 0.400000, 0.900000, 0.700000), // F
	color(1, 0.400000, 0.900000, 1.000000),
	color(1, 0.400000, 0.700000, 1.000000), // G
	color(1, 0.400000, 0.400000, 1.000000),
	color(1, 0.700000, 0.400000, 1.000000), // A
	color(1, 1.000000, 0.400000, 1.000000),
	color(1, 1.000000, 0.400000, 0.700000)  // B
};

color col_inter(const color a, const color &b, float t) {
	float e = 0.7f;
	color c;
	c.a = 1;//(1-t) * a.a + t * b.a;
	c.r = pow((1-t) * pow(a.r, e) + t * pow(b.r, e), 1/e);
	c.g = pow((1-t) * pow(a.g, e) + t * pow(b.g, e), 1/e);
	c.b = pow((1-t) * pow(a.b, e) + t * pow(b.b, e), 1/e);
	return c;
}

ColorScheme ColorScheme::disabled() const {
	ColorScheme c = *this;
	c.text = color::interpolate(text, background, 0.3f);
	c.selection = color::interpolate(selection, background, pow(0.3f, gamma));
	c.hover = color::interpolate(hover, background, 0.3f);
	c.auto_generate();
	return c;
}

void ColorScheme::auto_generate(bool keep_soft_text) {

	if (text.r > background.r) {
		high_contrast_a = White;
		high_contrast_b = Black;
	} else {
		high_contrast_a = Black;
		high_contrast_b = White;
	}

	//background_track_selected = color::interpolate(background, selection*1.5f, 0.17f);
	//background_track = color::interpolate(background, background_track_selected, 0.5f);
	background_track_selected = col_inter(background, selection*1.2f, 0.10f);
	background_track_selection = col_inter(background, selection*1.2f, 0.4f);
	background_track = background;

	selection_bars = selection;
	selection_bars.a = pow(0.13f, gamma);
	selection_bars_hover = color::interpolate(selection_bars, hover, 0.5f);
	selection_internal = selection;
	selection_internal.a = pow(0.1f, gamma);
	selection_boundary = selection;
	selection_boundary_hover = color::interpolate(selection, hover, 0.6f);
	preview_marker = color(1, 0, 0.7f, 0);
	preview_marker_internal = color(0.25f, 0, 0.7f, 0);
	capture_marker = color(1, 0.7f, 0, 0);
	if (!keep_soft_text) {
		text_soft1 = col_inter(background, text, 0.82f);
		text_soft3 = col_inter(background, col_inter(text, selection, 0.2f), 0.5f);
		text_soft2 = col_inter(text_soft3, text_soft1, 0.4f);
	}
	//grid = col_inter(background, col_inter(text, selection, 0.4f), 0.35f);
	grid = text_soft3;// col_inter(background, text_soft1, 0.35f);
	grid_selected = col_inter(grid, selection, 0.7f);

	//grid = text_soft3;
	sample = text_soft2;
	sample_selected = selection;

	background_overlay = color::interpolate(background, text_soft3, 0.1f);
	background_overlay.a = 0.8f;
	

	blob_bg_hidden = color::interpolate(color::interpolate(selection, text_soft3, 0.6f), background_track_selected, 0.8f);
	blob_bg = color::interpolate(color::interpolate(selection, text_soft2, 0.5f), background_track_selected, 0.6f);
	blob_bg_selected = color::interpolate(color::interpolate(selection, text_soft2, 0.4f), background_track_selected, 0.3f);
	color col_base_alt = color::interpolate(selection, Green, 0.8f);
	blob_bg_alt_hidden = color::interpolate(color::interpolate(col_base_alt, text_soft3, 0.6f), background_track_selected, 0.8f);
	blob_bg_alt = color::interpolate(color::interpolate(col_base_alt, text_soft2, 0.5f), background_track_selected, 0.6f);
	blob_bg_alt_selected = color::interpolate(color::interpolate(col_base_alt, text_soft2, 0.4f), background_track_selected, 0.3f);
	
	//selection_internal = blob_bg_selected;
	//grid_selected = blob_bg_selected;
	background_track_selection = blob_bg;

	red = color(1, 0.8f, 0, 0);
	green = color(1, 0, 0.7f, 0);
	blue = color(1, 0.1f, 0.1f, 1);
	white = White;

	for (int i=0; i<12; i++) {
		pitch[i] = PITCH_COLORS[i];
		pitch_text[i] = color::interpolate(pitch[i], text, 0.2f);
		pitch_soft1[i] = color::interpolate(pitch[i], background, 0.5f);
		pitch_soft2[i] = color::interpolate(pitch[i], background, 0.75f);
	}
}

color ColorScheme::pitch_color(int p) {
	return PITCH_COLORS[p % 12];
}

color ColorScheme::hoverify(const color &c) const {
	return col_inter(c, hover, 0.15f);
	return color::interpolate(c, hover, 0.1f);
}

ColorSchemeBright::ColorSchemeBright() {
	background = White;
	text = color(1, 0.15f, 0.15f, 0.15f);
	selection = color(1, 0.35f, 0.35f, 0.8f);
	hover = White;
	gamma = 1.0f;
	name = "bright";
	auto_generate();
}

ColorSchemeDark::ColorSchemeDark() {
	background = color(1, 0.15f, 0.15f, 0.15f);
	text = color(1, 0.95f, 0.95f, 0.95f);
	selection = color(1, 0.3f, 0.3f, 0.9f);
	hover = White;
	gamma = 0.3f;
	name = "dark";
	auto_generate();
}

ColorSchemeSystem::ColorSchemeSystem(hui::Panel *p, const string &id) {
	//{"base_color", "text_color", "fg_color", "bg_color", "selected_fg_color", "selected_bg_color", "insensitive_fg_color", "insensitive_bg_color", "borders", "unfocused_borders"};
	base::map<string,color> colors;
	hui::get_style_colors(p, id, colors);
	//background = colors["bg_color"];
	background = colors["base_color"];
	text = colors["text_color"];
	text = colors["fg_color"];
	text = colors["selected_fg_color"];

	gamma = 1.0f;
	if (text.r > background.r) // dark theme
		gamma = 0.3f;

	text_soft1 = colors["fg_color"];
	if (text == text_soft1) {
		text_soft1 = col_inter(background, text, 0.67f);
	}
	text_soft2 = col_inter(background, text_soft1, 0.7f);
	text_soft3 = col_inter(background, text_soft1, 0.4f);
	//selection = colors["selected_fg_color"];
	selection = color(1, 0.3f, 0.3f, 0.9f);
	//this->background
	hover = White;
	name = "system";
	auto_generate(true);
}
