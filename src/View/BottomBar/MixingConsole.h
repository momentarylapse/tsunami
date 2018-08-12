/*
 * MixingConsole.h
 *
 *  Created on: 16.03.2014
 *      Author: michi
 */

#ifndef MIXINGCONSOLE_H_
#define MIXINGCONSOLE_H_


#include "BottomBar.h"
class Track;
class Slider;
class Song;
class MixingConsole;
class PeakMeterDisplay;
class DeviceManager;
class AudioView;
class AudioViewTrack;

class TrackMixer;

class MixingConsole: public BottomBar::Console
{
public:
	MixingConsole(Session *session);
	virtual ~MixingConsole();

	void loadData();

	void onOutputVolume();

	void on_tracks_change();
	void on_solo_change();
	void onUpdateDeviceManager();

	virtual void _cdecl onShow();
	virtual void _cdecl onHide();

	DeviceManager *device_manager;
	PeakMeterDisplay *peak_meter;

	string id_inner;
	Array<TrackMixer*> mixer;
};

#endif /* MIXINGCONSOLE_H_ */
