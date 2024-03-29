/*
 * ViewModeScaleMarker.h
 *
 *  Created on: 26.05.2019
 *      Author: michi
 */

#ifndef SRC_VIEW_MODE_VIEWMODESCALEMARKER_H_
#define SRC_VIEW_MODE_VIEWMODESCALEMARKER_H_


#include "ViewModeDefault.h"
#include "../../data/Range.h"
#include "../../lib/base/base.h"

class TrackMarker;
class TrackLayer;

class ViewModeScaleMarker : public ViewModeDefault {
public:
	ViewModeScaleMarker(AudioView *view);

	void on_start() override;

	void on_left_button_up() override;
	void on_right_button_down() override;
	void on_mouse_move() override;
	void on_key_down(int k) override;

	void draw_post(Painter *c) override;
	string get_tip() override;

	void perform_scale();

	bool scaling;
	TrackLayer *layer;
	TrackMarker *marker;
};


#endif /* SRC_VIEW_MODE_VIEWMODESCALEMARKER_H_ */
