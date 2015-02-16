/*
 * MiniBar.h
 *
 *  Created on: 23.03.2014
 *      Author: michi
 */

#ifndef MINIBAR_H_
#define MINIBAR_H_

#include "../../lib/hui/hui.h"
#include "../../Stuff/Observer.h"

class PeakMeter;
class AudioStream;
class BottomBar;

class MiniBar : public HuiPanel, public Observer
{
public:
	MiniBar(BottomBar *bottom_bar, AudioStream *stream);
	virtual ~MiniBar();

	void onShowBottomBar();

	virtual void onShow();
	virtual void onHide();

	virtual void onUpdate(Observable *o, const string &message);

	AudioStream *stream;
	PeakMeter *peak_meter;
	BottomBar *bottom_bar;
};

#endif /* MINIBAR_H_ */
