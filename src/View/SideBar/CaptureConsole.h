/*
 * CaptureDialog.h
 *
 *  Created on: 27.03.2012
 *      Author: michi
 */

#ifndef SRC_VIEW_SIDEBAR_CAPTURECONSOLE_H_
#define SRC_VIEW_SIDEBAR_CAPTURECONSOLE_H_


#include "SideBar.h"
#include "../Helper/PeakMeter.h"
#include "../../Stuff/Observer.h"
#include "../../Data/Song.h"

class AudioView;
class DeviceManager;
class Device;
class CaptureConsoleMode;

class CaptureConsole : public SideBarConsole, public Observer
{
public:
	CaptureConsole(Song *s, AudioView *view);
	virtual ~CaptureConsole();



	virtual void onEnter();
	virtual void onLeave();

	void onType();

	void onStart();
	void onDelete();
	void onPause();
	void onOk();
	void onCancel();
	void onClose();

	DeviceManager *device_manager;

	void updateTime();

	void onUpdate(Observable *o, const string &message);

	bool isCapturing();

	Song *song;
	AudioView *view;
	PeakMeter *peak_meter;
	CaptureConsoleMode *mode;

	CaptureConsoleMode *mode_audio;
	CaptureConsoleMode *mode_midi;
	CaptureConsoleMode *mode_multi;
};

#endif /* SRC_VIEW_SIDEBAR_CAPTURECONSOLE_H_ */
