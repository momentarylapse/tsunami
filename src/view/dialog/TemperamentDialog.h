/*
 * TemperamentDialog.h
 *
 *  Created on: 25.12.2015
 *      Author: michi
 */

#ifndef SRC_VIEW_DIALOG_TEMPERAMENTDIALOG_H_
#define SRC_VIEW_DIALOG_TEMPERAMENTDIALOG_H_

#include "../../lib/hui/hui.h"
#include "../../data/midi/Temperament.h"

namespace tsunami {

class Track;
class AudioView;

class TemperamentDialog : public hui::Dialog {
public:
	TemperamentDialog(Track *t, AudioView *view, hui::Window *parent);

	void _cdecl on_draw(Painter *p) override;
	void _cdecl on_left_button_down(const vec2& m) override;
	void _cdecl on_left_button_up(const vec2& m) override;
	void _cdecl on_mouse_move(const vec2& m) override;
	void _cdecl on_mouse_wheel(const vec2& d) override;


	void on_close();
	void on_ok();
	void on_relative();
	void on_all_octaves();
	void on_preset();
	void on_reference_pitch();
	void on_reference_freq();

	void apply_preset();

	float pitch2x(float p);
	float pitch2y(float p);
	float relpitch2y(float p, float p0);
	float x2pitch(float x);
	float y2pitch(float y);
	float y2relpitch(float y, float p0);

	Track *track;
	AudioView *view;
	float width, height;

	bool mode_relative;
	bool all_octaves;

	Temperament temperament;

	int hover;
};

}

#endif /* SRC_VIEW_DIALOG_TEMPERAMENTDIALOG_H_ */
