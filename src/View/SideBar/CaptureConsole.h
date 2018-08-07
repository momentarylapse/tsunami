/*
 * CaptureDialog.h
 *
 *  Created on: 27.03.2012
 *      Author: michi
 */

#ifndef SRC_VIEW_SIDEBAR_CAPTURECONSOLE_H_
#define SRC_VIEW_SIDEBAR_CAPTURECONSOLE_H_


#include "SideBar.h"
#include "../../Data/Song.h"
#include "../Helper/PeakMeterDisplay.h"

class AudioView;
class DeviceManager;
class Device;
class CaptureConsoleMode;
class Session;

class CaptureConsole : public SideBarConsole
{
public:
	CaptureConsole(Session *session);
	virtual ~CaptureConsole();



	void on_enter() override;
	void on_leave() override;

	void onStart();
	void onDump();
	void onPause();
	void onOk();
	void onCancel();
	void onClose();
	void onNewVersion();

	DeviceManager *device_manager;

	void updateTime();

	void onOutputUpdate();
	void onOutputEndOfStream();

	bool isCapturing();

	PeakMeterDisplay *peak_meter;
	CaptureConsoleMode *mode;

	CaptureConsoleMode *mode_audio;
	CaptureConsoleMode *mode_midi;
	CaptureConsoleMode *mode_multi;
};

#endif /* SRC_VIEW_SIDEBAR_CAPTURECONSOLE_H_ */
