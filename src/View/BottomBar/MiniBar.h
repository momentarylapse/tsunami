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
class OutputStream;
class DeviceManager;
class BottomBar;
class AudioView;

class MiniBar : public HuiPanel, public Observer
{
public:
	MiniBar(BottomBar *bottom_bar, OutputStream *stream, DeviceManager *output, AudioView *view);
	virtual ~MiniBar();

	void onShowBottomBar();
	void onVolume();

	virtual void onShow();
	virtual void onHide();

	virtual void onUpdate(Observable *o, const string &message);

	OutputStream *stream;
	DeviceManager *output;
	PeakMeter *peak_meter;
	BottomBar *bottom_bar;
};

#endif /* MINIBAR_H_ */
