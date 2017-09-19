/*
 * ViewModeCapture.h
 *
 *  Created on: 14.02.2016
 *      Author: michi
 */

#ifndef SRC_VIEW_MODE_VIEWMODECAPTURE_H_
#define SRC_VIEW_MODE_VIEWMODECAPTURE_H_

#include "ViewModeDefault.h"
#include "../../Data/Range.h"
#include "../../lib/base/base.h"

class InputStreamAudio;
class InputStreamMidi;

//class CaptureConsoleMode;

class ViewModeCapture : public ViewModeDefault
{
public:
	ViewModeCapture(AudioView *view);
	virtual ~ViewModeCapture();

	virtual Selection getHover();
	virtual void onLeftButtonDown(){}
	virtual void onLeftDoubleClick(){}

	virtual void drawPost(Painter *c);

	//CaptureConsoleMode *console_mode;

	InputStreamAudio *input_audio;
	InputStreamMidi *input_midi;
	void setInputAudio(InputStreamAudio *input);
	void setInputMidi(InputStreamMidi *input);
	Track *capturing_track;

	void onInputUpdate();
};

#endif /* SRC_VIEW_MODE_VIEWMODECAPTURE_H_ */
