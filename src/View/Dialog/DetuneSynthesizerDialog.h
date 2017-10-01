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

	virtual void _cdecl onDraw(Painter *p);
	virtual void _cdecl onLeftButtonDown();
	virtual void _cdecl onLeftButtonUp();
	virtual void _cdecl onMouseMove();
	virtual void _cdecl onMouseWheel();


	void onClose();
	void onRelative();

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
