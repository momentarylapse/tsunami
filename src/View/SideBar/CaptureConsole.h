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
class InputStreamAny;
class DeviceManager;
class Device;

class CaptureConsole : public SideBarConsole, public Observer
{
public:
	CaptureConsole(Song *s, AudioView *view);
	virtual ~CaptureConsole();



	virtual void onEnter();
	virtual void onLeave();

	void beginAudio();
	void beginMidi();
	void beginMulti();
	void beginMode(int type);
	void endCapture();

	void onType();

	void onSourceAudio();
	void onTargetAudio();
	void onSourceMidi();
	void onTargetMidi();

	void onStart();
	void onDelete();
	void onPause();
	void onOk();
	void onCancel();
	void onClose();
	bool insert();
	bool insertAudio();
	bool insertMidi();
	bool insertMulti();

	void fillTrackList();

	DeviceManager *device_manager;

	void updateSourceList();
	Array<Device*> sources_audio;
	Device *chosen_device_audio;
	Array<Device*> sources_midi;
	Device *chosen_device_midi;
	void updateTime();

	void onUpdate(Observable *o, const string &message);

	void setTargetAudio(int index);
	void setTargetMidi(int index);
	//void setType(int type);

	Song *song;
	AudioView *view;
	InputStreamAny *input;
	//InputStreamAny *input_midi;
	PeakMeter *peak_meter;
	Synthesizer *temp_synth;
	int type;

	int multi_size;
};

#endif /* SRC_VIEW_SIDEBAR_CAPTURECONSOLE_H_ */
