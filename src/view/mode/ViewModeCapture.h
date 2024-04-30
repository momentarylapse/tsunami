/*
 * ViewModeCapture.h
 *
 *  Created on: 14.02.2016
 *      Author: michi
 */

#ifndef SRC_VIEW_MODE_VIEWMODECAPTURE_H_
#define SRC_VIEW_MODE_VIEWMODECAPTURE_H_

#include "ViewModeDefault.h"

class SignalChain;
class CaptureTrackData;

class ViewModeCapture : public ViewModeDefault {
public:
	ViewModeCapture(AudioView *view);

	void on_start() override;
	void on_end() override;

	void on_left_button_down(const vec2 &m) override {}
	void on_left_double_click(const vec2 &m) override {}

	void draw_post(Painter *c) override;


	virtual base::set<Track*> prevent_playback() override;

	Array<CaptureTrackData> data;
	SignalChain *chain;
	
	void insert();
};

#endif /* SRC_VIEW_MODE_VIEWMODECAPTURE_H_ */
