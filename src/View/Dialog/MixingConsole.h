/*
 * MixingConsole.h
 *
 *  Created on: 16.03.2014
 *      Author: michi
 */

#ifndef MIXINGCONSOLE_H_
#define MIXINGCONSOLE_H_


#include "../Helper/EmbeddedDialog.h"
#include "../../Stuff/Observer.h"
class Track;
class Slider;
class AudioFile;

class TrackMixer: public EmbeddedDialog
{
public:
	TrackMixer(int index, HuiWindow *win);
	~TrackMixer();
	void OnVolume();
	void OnMute();
	void OnPanning();
	void SetTrack(Track *t);
	void Update();

	static const float DB_MIN;
	static const float DB_MAX;
	static const float TAN_SCALE;
	static float db2slider(float db);
	static float slider2db(float val);
	static float vol2slider(float vol);
	static float slider2vol(float val);

	int index;
	Track *track;
	//Slider *volume_slider;
	//Slider *panning_slider;
	string id_grid;
	string id_name;
	string vol_slider_id;
	string pan_slider_id;
	string mute_id;
};

class MixingConsole: public EmbeddedDialog, public Observer, public Observable
{
public:
	MixingConsole(AudioFile *audio, HuiWindow *win);
	virtual ~MixingConsole();

	void LoadData();

	void Show(bool show);

	virtual void OnUpdate(Observable *o);

	AudioFile *audio;

	Array<TrackMixer*> mixer;
	bool enabled;
};

#endif /* MIXINGCONSOLE_H_ */
