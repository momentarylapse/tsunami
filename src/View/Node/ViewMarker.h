/*
 * ViewMarker.h
 *
 *  Created on: 04.07.2019
 *      Author: michi
 */

#ifndef SRC_VIEW_NODE_VIEWMARKER_H_
#define SRC_VIEW_NODE_VIEWMARKER_H_

#include "ViewNode.h"

class TrackMarker;
class AudioViewTrack;

class ViewMarker : public ViewNode {
public:
	ViewMarker(AudioViewTrack *parent, TrackMarker *marker);
	~ViewMarker();

	void draw(Painter *c) override;
	HoverData get_hover_data(float mx, float my) override;

	bool on_left_button_down() override;
	bool on_left_double_click() override;
	bool on_right_button_down() override;

	TrackMarker *marker;
};

#endif // SRC_VIEW_NODE_VIEWMARKER_H_
