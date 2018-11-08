/*
 * DetuneSynthesizerDialog.h
 *
 *  Created on: 25.12.2015
 *      Author: michi
 */

#ifndef SRC_VIEW_DIALOG_DETUNESYNTHESIZERDIALOG_H_
#define SRC_VIEW_DIALOG_DETUNESYNTHESIZERDIALOG_H_

#include "../../lib/hui/hui.h"

class Synthesizer;
class Track;
class AudioView;

class DetuneSynthesizerDialog : public hui::Dialog
{
public:
	DetuneSynthesizerDialog(Synthesizer *s, Track *t, AudioView *view, hui::Window *parent);
	virtual ~DetuneSynthesizerDialog();

	void _cdecl on_draw(Painter *p) override;
	void _cdecl on_left_button_down() override;
	void _cdecl on_left_button_up() override;
	void _cdecl on_mouse_move() override;
	void _cdecl on_mouse_wheel() override;


	void on_close();
	void on_relative();

	float pitch2x(float p);
	float pitch2y(float p);
	float relpitch2y(float p, float p0);

	Synthesizer *synth;
	Track *track;
	AudioView *view;
	float width, height;

	bool mode_relative;

	int hover;
};

#endif /* SRC_VIEW_DIALOG_DETUNESYNTHESIZERDIALOG_H_ */
