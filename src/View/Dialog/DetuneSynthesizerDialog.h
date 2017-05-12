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
	float pitch2x(float p);

	float freq2y(float f);
	float y2freq(float y);

	Synthesizer *synth;
	Track *track;
	AudioView *view;
	float width, height;

	int hover;
};

#endif /* SRC_VIEW_DIALOG_DETUNESYNTHESIZERDIALOG_H_ */
