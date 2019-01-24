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

	void on_show_bottom_bar();
	void on_volume();
	void on_selection_snap_mode();

	void _cdecl on_show() override;
	void _cdecl on_hide() override;

	void on_bottom_bar_update();
	void on_volume_change();
	void on_view_settings_change();

	//OutputStream *stream;
	Session *session;
	DeviceManager *dev_manager;
	PeakMeterDisplay *peak_meter;
	CpuDisplay *cpu_display;
	BottomBar *bottom_bar;
	AudioView *view;
};

#endif /* SRC_VIEW_BOTTOMBAR_MINIBAR_H_ */
