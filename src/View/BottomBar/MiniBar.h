/*
 * MiniBar.h
 *
 *  Created on: 23.03.2014
 *      Author: michi
 */

#ifndef SRC_VIEW_BOTTOMBAR_MINIBAR_H_
#define SRC_VIEW_BOTTOMBAR_MINIBAR_H_

#include "../../lib/hui/hui.h"

class PeakMeterDisplay;
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
	PeakMeterDisplay *peak_meter;
	CpuDisplay *cpu_display;
	BottomBar *bottom_bar;
	AudioView *view;
};

#endif /* SRC_VIEW_BOTTOMBAR_MINIBAR_H_ */
