/*
 * MixingConsole.cpp
 *
 *  Created on: 16.03.2014
 *      Author: michi
 */

#include "MixingConsole.h"
#include "../../Device/OutputStream.h"
#include "../Helper/PeakMeterDisplay.h"
#include <math.h>

#include "../../Device/DeviceManager.h"
#include "../../Data/base.h"
#include "../../Data/Song.h"
#include "../../Data/Track.h"
#include "../../Session.h"
#include "../AudioView.h"
#include "../AudioViewTrack.h"

class TrackMixer: public hui::Panel
{
public:
	TrackMixer(AudioViewTrack *t)
	{
		fromResource("track-mixer");

		track = nullptr;
		vtrack = nullptr;
		editing = false;

		id_separator = "mixing-track-separator";
		id_name = "name";
		vol_slider_id = "volume";
		pan_slider_id = "panning";
		mute_id = "mute";
		addString(pan_slider_id, "-1\\L");
		addString(pan_slider_id, "0\\");
		addString(pan_slider_id, "1\\R");
		addString(vol_slider_id, format("%f\\%+d", db2slider(DB_MAX), (int)DB_MAX));
		addString(vol_slider_id, format("%f\\%+d", db2slider(5), (int)5));
		addString(vol_slider_id, format("%f\\%d", db2slider(0), 0));
		addString(vol_slider_id, format("%f\\%d", db2slider(-5), (int)-5));
		addString(vol_slider_id, format("%f\\%d", db2slider(-10), (int)-10));
		addString(vol_slider_id, format("%f\\%d", db2slider(-20), (int)-20));
		addString(vol_slider_id, format("%f\\-\u221e", db2slider(DB_MIN))); // \u221e

		event(vol_slider_id, std::bind(&TrackMixer::on_volume, this));
		event(pan_slider_id, std::bind(&TrackMixer::on_panning, this));
		event(mute_id, std::bind(&TrackMixer::on_mute, this));
		event("solo", std::bind(&TrackMixer::on_solo, this));

		set_track(t);
	}
	~TrackMixer()
	{
		set_track(nullptr);
	}

	void on_volume()
	{
		editing = true;
		if (track){
			if (parent->isChecked("link-volumes"))
				track->song->changeAllTrackVolumes(track, slider2vol(getFloat("")));
			else
				vtrack->setVolume(slider2vol(getFloat("")));
		}
		editing = false;
	}

	// allow update() for mute/solo!
	void on_mute()
	{
		if (vtrack)
			vtrack->setMuted(isChecked(""));
	}
	void on_solo()
	{
		if (vtrack)
			vtrack->setSolo(isChecked(""));
	}

	void on_panning()
	{
		editing = true;
		if (vtrack)
			vtrack->setPanning(getFloat(""));
		editing = false;
	}
	void set_track(AudioViewTrack *t)
	{
		if (track)
			track->unsubscribe(this);
		if (vtrack)
			vtrack->unsubscribe(this);
		vtrack = t;
		track = nullptr;
		if (t)
			track = t->track;
		if (track)
			track->subscribe(this, [&]{ update(); }, track->MESSAGE_CHANGE);
		if (vtrack){
			vtrack->subscribe(this, [&]{ update(); }, vtrack->MESSAGE_CHANGE);
			vtrack->subscribe(this, [&]{ on_vtrack_delete(); }, track->MESSAGE_DELETE);
		}
		update();
	}
	void on_vtrack_delete()
	{
		set_track(nullptr);

	}
	void update()
	{
		if (!vtrack)
			return;
		if (editing)
			return;
		setFloat(vol_slider_id, vol2slider(track->volume));
		setFloat(pan_slider_id, track->panning);
		check(mute_id, track->muted);
		check("solo", vtrack->solo);
		bool is_playable = vtrack->view->get_playable_tracks().contains(track);
		enable(id_name, is_playable);
		if (is_playable)
			setString(id_name, track->getNiceName());
		else
			setString(id_name, "<s>" + track->getNiceName() + "</s>");
	}


	static constexpr float DB_MIN = -1000000;
	static constexpr float DB_MAX = 10;
	static constexpr float TAN_SCALE = 10.0f;

	static float db2slider(float db)
	{
		return (atan(db / TAN_SCALE) - atan(DB_MIN / TAN_SCALE)) / (atan(DB_MAX / TAN_SCALE) - atan(DB_MIN / TAN_SCALE));
	}
	static float slider2db(float val)
	{
		return tan(atan(DB_MIN / TAN_SCALE) + val * (atan(DB_MAX / TAN_SCALE)- atan(DB_MIN / TAN_SCALE))) * TAN_SCALE;
	}
	static float vol2slider(float vol)
	{
		return db2slider(amplitude2db(vol));
	}
	static float slider2vol(float val)
	{
		return db2amplitude(slider2db(val));
	}


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
	bool editing;
};




MixingConsole::MixingConsole(Session *session) :
	BottomBar::Console(_("Mixing console"), session)
{
	device_manager = session->device_manager;
	id_inner = "inner-grid";

	fromResource("mixing-console");

	peak_meter = new PeakMeterDisplay(this, "output-peaks", view->peak_meter, view);
	setFloat("output-volume", device_manager->getOutputVolume());

	event("output-volume", std::bind(&MixingConsole::on_output_volume, this));

	view->subscribe(this, std::bind(&MixingConsole::on_tracks_change, this), session->view->MESSAGE_VTRACK_CHANGE);
	view->subscribe(this, [&]{ update_all(); }, session->view->MESSAGE_SOLO_CHANGE);
	song->subscribe(this, [&]{ update_all(); }, song->MESSAGE_FINISHED_LOADING);

	//song->subscribe(this, std::bind(&MixingConsole::onUpdateSong, this));
	device_manager->subscribe(this, std::bind(&MixingConsole::on_update_device_manager, this));
	load_data();
}

MixingConsole::~MixingConsole()
{
	//song->unsubscribe(this);
	view->unsubscribe(this);
	device_manager->unsubscribe(this);
	for (TrackMixer *m: mixer)
		delete(m);
	delete(peak_meter);
}

void MixingConsole::on_output_volume()
{
	device_manager->setOutputVolume(getFloat(""));
}

void MixingConsole::load_data()
{
	// how many TrackMixers still match?
	int n_ok = 0;
	foreachi (auto *m, mixer, i){
		if (!m->vtrack)
			break;
		if (i >= view->vtrack.num)
			break;
		if (m->vtrack != view->vtrack[i])
			break;
		n_ok ++;
	}

	// delete non-matching
	for (int i=n_ok; i<mixer.num; i++){
		delete mixer[i];
		removeControl("separator-" + i2s(i));
	}
	mixer.resize(n_ok);

	// add new
	foreachi(AudioViewTrack *t, view->vtrack, i){
		if (i >= n_ok){
			TrackMixer *m = new TrackMixer(t);
			mixer.add(m);
			embed(m, id_inner, i*2, 0);
			addSeparator("!vertical", i*2 + 1, 0, "separator-" + i2s(i));
		}
	}

	hideControl("link-volumes", mixer.num <= 1);
}

void MixingConsole::on_update_device_manager()
{
	setFloat("output-volume", device_manager->getOutputVolume());
}

void MixingConsole::on_tracks_change()
{
	load_data();
}

void MixingConsole::update_all()
{
	for (auto *m: mixer)
		m->update();
}

void MixingConsole::onShow()
{
	peak_meter->enable(true);
}

void MixingConsole::onHide()
{
	peak_meter->enable(false);
}
