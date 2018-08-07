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

class TrackMixer: public hui::Panel
{
public:
	TrackMixer();
	~TrackMixer();
	void onVolume();
	void onMute();
	void onSolo();
	void onPanning();
	void setTrack(AudioViewTrack *t);
	void update();

	static const float DB_MIN;
	static const float DB_MAX;
	static const float TAN_SCALE;
	static float db2slider(float db);
	static float slider2db(float val);
	static float vol2slider(float vol);
	static float slider2vol(float val);

	Track *track;
	AudioViewTrack *vtrack;
	//Slider *volume_slider;
	//Slider *panning_slider;
	string id_name;
	string vol_slider_id;
	string pan_slider_id;
	string mute_id;
	string id_separator;
	AudioView *view;
};

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
