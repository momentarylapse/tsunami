/*
 * MixingConsole.cpp
 *
 *  Created on: 16.03.2014
 *      Author: michi
 */

#include "MixingConsole.h"
#include "../../Device/OutputStream.h"
#include "../Helper/PeakMeter.h"
#include <math.h>

#include "../../Device/DeviceManager.h"
#include "../../Data/Song.h"

const float TrackMixer::DB_MIN = -1000000;
const float TrackMixer::DB_MAX = 10;
const float TrackMixer::TAN_SCALE = 10.0f;

TrackMixer::TrackMixer()
{
	fromResource("track-mixer");

	id_separator = "mixing-track-separator";
	id_name = "name";
	vol_slider_id = "volume";
	pan_slider_id = "panning";
	mute_id = "mute";
	addString(pan_slider_id, "0\\L");
	addString(pan_slider_id, "0.5\\");
	addString(pan_slider_id, "1\\R");
	addString(vol_slider_id, format("%f\\%+d", db2slider(DB_MAX), (int)DB_MAX));
	addString(vol_slider_id, format("%f\\%+d", db2slider(5), (int)5));
	addString(vol_slider_id, format("%f\\%d", db2slider(0), 0));
	addString(vol_slider_id, format("%f\\%d", db2slider(-5), (int)-5));
	addString(vol_slider_id, format("%f\\%d", db2slider(-10), (int)-10));
	addString(vol_slider_id, format("%f\\%d", db2slider(-20), (int)-20));
	addString(vol_slider_id, format("%f\\-inf", db2slider(DB_MIN))); // \u221e

	event(vol_slider_id, std::bind(&TrackMixer::onVolume, this));
	event(pan_slider_id, std::bind(&TrackMixer::onPanning, this));
	event(mute_id, std::bind(&TrackMixer::onMute, this));
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
	if (parent->isChecked("link-volumes"))
		track->song->changeAllTrackVolumes(track, slider2vol(getFloat("")));
	else
		track->setVolume(slider2vol(getFloat("")));
}

void TrackMixer::onMute()
{
	track->setMuted(isChecked(""));
}

void TrackMixer::onPanning()
{
	track->setPanning(getFloat("")*2 - 1);
}

void TrackMixer::setTrack(Track* t)
{
	track = t;
	update();
}

void TrackMixer::update()
{
	setFloat(vol_slider_id, vol2slider(track->volume));
	setFloat(pan_slider_id, track->panning * 0.5f + 0.5f);
	check(mute_id, track->muted);
	setString(id_name, "!bold\\" + track->getNiceName());
}


MixingConsole::MixingConsole(Song *_song, DeviceManager *_device_manager, OutputStream *stream, AudioView *view) :
	BottomBar::Console(_("Mixing console")),
	Observer("MixingConsole")
{
	song = _song;
	device_manager = _device_manager;
	id_inner = "inner-grid";

	fromResource("mixing-console");

	peak_meter = new PeakMeter(this, "output-peaks", stream, view);
	setFloat("output-volume", device_manager->getOutputVolume());

	event("output-volume", std::bind(&MixingConsole::onOutputVolume, this));

	song->subscribe(this);
	device_manager->subscribe(this);
	loadData();
}

MixingConsole::~MixingConsole()
{
	song->unsubscribe(this);
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
	for (int i=mixer.num; i<song->tracks.num; i++){
		TrackMixer *m = new TrackMixer();
		mixer.add(m);
		embed(m, id_inner, i*2, 0);
		addSeparator("!vertical", i*2 + 1, 0, 0, 0, "separator-" + i2s(i));
	}
	for (int i=song->tracks.num; i<mixer.num; i++){
		delete(mixer[i]);
		removeControl("separator-" + i2s(i));
	}
	mixer.resize(song->tracks.num);

	foreachi(Track *t, song->tracks, i)
		mixer[i]->setTrack(t);

	hideControl("link-volumes", song->tracks.num <= 1);
}

void MixingConsole::onUpdate(Observable* o, const string &message)
{
	if (o == device_manager)
		setFloat("output-volume", device_manager->getOutputVolume());
	else
		loadData();
}

void MixingConsole::onShow()
{
	peak_meter->enable(true);
}

void MixingConsole::onHide()
{
	peak_meter->enable(false);
}
