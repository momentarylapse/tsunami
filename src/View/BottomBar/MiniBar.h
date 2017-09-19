/*
 * MiniBar.h
 *
 *  Created on: 23.03.2014
 *      Author: michi
 */

#ifndef MINIBAR_H_
#define MINIBAR_H_

#include "../../lib/hui/hui.h"

class PeakMeter;
class OutputStream;
class DeviceManager;
class BottomBar;
class AudioView;

class MiniBar : public hui::Panel
{
public:
	MiniBar(BottomBar *bottom_bar, DeviceManager *dev_manager, AudioView *view);
	virtual ~MiniBar();

	void onShowBottomBar();
	void onVolume();

	virtual void _cdecl onShow();
	virtual void _cdecl onHide();

	void onBottomBarUpdate();
	void onVolumeChange();

	void onViewOutputChange();

	//OutputStream *stream;
	DeviceManager *dev_manager;
	PeakMeter *peak_meter;
	BottomBar *bottom_bar;
	AudioView *view;
};

#endif /* MINIBAR_H_ */
