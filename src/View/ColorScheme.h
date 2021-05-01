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

namespace hui {
	class Panel;
}

class ColorScheme {
public:

	void auto_generate();
	ColorScheme disabled() const;

	// basic
	color background;
	color text;
	color selection;
	color hover;
	float gamma;
	string name;

	color background_track;
	color background_track_selected;
	color background_track_selection;
	color background_overlay;
	color selection_internal;
	color selection_boundary;
	color selection_boundary_hover;
	color selection_bars;
	color selection_bars_hover;
	color preview_marker;
	color preview_marker_internal;
	color capture_marker;
	color text_soft1;
	color text_soft2;
	color text_soft3;
	color grid;
	color grid_selected;
	color sample;
	color sample_selected;
	color high_contrast_a, high_contrast_b;
	

	color blob_bg, blob_bg_hidden, blob_bg_selected;
	color blob_bg_alt, blob_bg_alt_hidden, blob_bg_alt_selected;

	color red, blue, green, white;

	/*color layer(bool selected, bool hover);
	color sample(bool selected, bool hover);
	color buffer(bool selected, bool hover);
	color layer_header(bool selected, bool hover);*/
	
	color hoverify(const color &c) const;
};

class ColorSchemeBright : public ColorScheme {
public:
	ColorSchemeBright();
};

class ColorSchemeDark : public ColorScheme {
public:
	ColorSchemeDark();
};

class ColorSchemeSystem : public ColorScheme {
public:
	ColorSchemeSystem(hui::Panel *p, const string &id);
};

#endif /* SRC_VIEW_COLORSCHEME_H_ */
