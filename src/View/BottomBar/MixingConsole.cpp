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

const float TrackMixer::DB_MIN = -1000000;
const float TrackMixer::DB_MAX = 10;
const float TrackMixer::TAN_SCALE = 10.0f;

TrackMixer::TrackMixer()
{
	fromResource("track-mixer");

	track = nullptr;
	vtrack = nullptr;

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

	event(vol_slider_id, std::bind(&TrackMixer::onVolume, this));
	event(pan_slider_id, std::bind(&TrackMixer::onPanning, this));
	event(mute_id, std::bind(&TrackMixer::onMute, this));
	event("solo", std::bind(&TrackMixer::onSolo, this));
}

TrackMixer::~TrackMixer()
{
}

float TrackMixer::slider2db(float val)
{
	return tan(atan(DB_MIN / TAN_SCALE) + val * (atan(DB_MAX / TAN_SCALE)- atan(DB_MIN / TAN_SCALE))) * TAN_SCALE;
}

float TrackMixer::db2slider(float db)
{
	return (atan(db / TAN_SCALE) - atan(DB_MIN / TAN_SCALE)) / (atan(DB_MAX / TAN_SCALE) - atan(DB_MIN / TAN_SCALE));
}

float TrackMixer::slider2vol(float val)
{
	return db2amplitude(slider2db(val));
}

float TrackMixer::vol2slider(float vol)
{
	return db2slider(amplitude2db(vol));
}

void TrackMixer::onVolume()
{
	if (track){
		if (parent->isChecked("link-volumes"))
			track->song->changeAllTrackVolumes(track, slider2vol(getFloat("")));
		else
			track->setVolume(slider2vol(getFloat("")));
	}
}

void TrackMixer::onMute()
{
	if (track)
		track->setMuted(isChecked(""));
}

void TrackMixer::onSolo()
{
	if (vtrack)
		vtrack->setSolo(isChecked(""));
}

void TrackMixer::onPanning()
{
	if (track)
		track->setPanning(getFloat(""));
}

void TrackMixer::setTrack(AudioViewTrack* t)
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
	if (vtrack)
		vtrack->subscribe(this, [&]{ update(); }, track->MESSAGE_CHANGE);
	update();
}

void TrackMixer::update()
{
	setFloat(vol_slider_id, vol2slider(track->volume));
	setFloat(pan_slider_id, track->panning);
	check(mute_id, track->muted);
	check("solo", vtrack->solo);
	setString(id_name, track->getNiceName());
	bool is_playable = vtrack->view->get_playable_tracks().contains(track);
	enable(id_name, is_playable);
}


MixingConsole::MixingConsole(Session *session) :
	BottomBar::Console(_("Mixing console"), session)
{
	device_manager = session->device_manager;
	id_inner = "inner-grid";

	fromResource("mixing-console");

	peak_meter = new PeakMeterDisplay(this, "output-peaks", view->peak_meter, view);
	setFloat("output-volume", device_manager->getOutputVolume());

	event("output-volume", std::bind(&MixingConsole::onOutputVolume, this));

	view->subscribe(this, std::bind(&MixingConsole::on_tracks_change, this), session->view->MESSAGE_VTRACK_CHANGE);
	view->subscribe(this, std::bind(&MixingConsole::on_solo_change, this), session->view->MESSAGE_SOLO_CHANGE);
	//song->subscribe(this, std::bind(&MixingConsole::onUpdateSong, this));
	device_manager->subscribe(this, std::bind(&MixingConsole::onUpdateDeviceManager, this));
	loadData();
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

void MixingConsole::onOutputVolume()
{
	device_manager->setOutputVolume(getFloat(""));
}

void MixingConsole::loadData()
{
	int n = view->vtrack.num;
	for (int i=mixer.num; i<n; i++){
		TrackMixer *m = new TrackMixer();
		mixer.add(m);
		embed(m, id_inner, i*2, 0);
		addSeparator("!vertical", i*2 + 1, 0, "separator-" + i2s(i));
	}
	for (int i=n; i<mixer.num; i++){
		delete(mixer[i]);
		removeControl("separator-" + i2s(i));
	}
	mixer.resize(n);

	foreachi(AudioViewTrack *t, view->vtrack, i)
		mixer[i]->setTrack(t);

	hideControl("link-volumes", n <= 1);
}

void MixingConsole::onUpdateDeviceManager()
{
	setFloat("output-volume", device_manager->getOutputVolume());
}

void MixingConsole::on_tracks_change()
{
	loadData();
}

void MixingConsole::on_solo_change()
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
