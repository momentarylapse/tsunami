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
class CpuDisplay;
class OutputStream;
class DeviceManager;
class BottomBar;
class AudioView;
class Session;

class MiniBar : public hui::Panel
{
public:
	MiniBar(BottomBar *bottom_bar, Session *session);
	virtual ~MiniBar();

	void onShowBottomBar();
	void onVolume();

	virtual void _cdecl onShow();
	virtual void _cdecl onHide();

	void onBottomBarUpdate();
	void onVolumeChange();

	//OutputStream *stream;
	Session *session;
	DeviceManager *dev_manager;
	PeakMeter *peak_meter;
	CpuDisplay *cpu_display;
	BottomBar *bottom_bar;
	AudioView *view;
};

#endif /* MINIBAR_H_ */
