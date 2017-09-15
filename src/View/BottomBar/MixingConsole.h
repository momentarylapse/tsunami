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
class PeakMeter;
class DeviceManager;
class OutputStream;
class AudioView;

class TrackMixer: public hui::Panel
{
public:
	TrackMixer();
	~TrackMixer();
	void onVolume();
	void onMute();
	void onPanning();
	void setTrack(Track *t);
	void update();

	static const float DB_MIN;
	static const float DB_MAX;
	static const float TAN_SCALE;
	static float db2slider(float db);
	static float slider2db(float val);
	static float vol2slider(float vol);
	static float slider2vol(float val);

	Track *track;
	//Slider *volume_slider;
	//Slider *panning_slider;
	string id_name;
	string vol_slider_id;
	string pan_slider_id;
	string mute_id;
	string id_separator;
};

class MixingConsole: public BottomBar::Console
{
public:
	MixingConsole(Song *audio, DeviceManager *device_manager, OutputStream *stream, AudioView *view);
	virtual ~MixingConsole();

	void loadData();

	void onOutputVolume();

	void onUpdateSong();
	void onUpdateDeviceManager();

	virtual void _cdecl onShow();
	virtual void _cdecl onHide();

	Song *song;
	DeviceManager *device_manager;
	PeakMeter *peak_meter;

	string id_inner;
	Array<TrackMixer*> mixer;
};

#endif /* MIXINGCONSOLE_H_ */
