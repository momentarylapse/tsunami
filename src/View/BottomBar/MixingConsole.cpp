/*
 * MixingConsole.cpp
 *
 *  Created on: 16.03.2014
 *      Author: michi
 */

#include "MixingConsole.h"
#include "../../Data/AudioFile.h"
#include "../../Audio/AudioOutput.h"
#include "../../Audio/AudioStream.h"
#include "../Helper/PeakMeter.h"
#include <math.h>

const float TrackMixer::DB_MIN = -1000000;
const float TrackMixer::DB_MAX = 10;
const float TrackMixer::TAN_SCALE = 10.0f;

TrackMixer::TrackMixer()
{
	string id_grid = "mixing_track_table";
	id_separator = "mixing_track_separator";
	AddControlTable("", 0, 0, 1, 4, id_grid);
	SetTarget(id_grid, 0);
	id_name = "track_name";
	AddText("!center\\...", 0, 0, 0, 0, id_name);
	vol_slider_id = "volume";
	pan_slider_id = "panning";
	mute_id = "mute";
	AddSlider("!width=80,origin=no", 0, 1, 0, 0, pan_slider_id);
	AddString(pan_slider_id, "0\\L");
	AddString(pan_slider_id, "0.5\\");
	AddString(pan_slider_id, "1\\R");
	SetTooltip(pan_slider_id, "Balance");
	AddSlider("!vertical,expandy", 0, 2, 0, 0, vol_slider_id);
	AddString(vol_slider_id, format("%f\\%+d", db2slider(DB_MAX), (int)DB_MAX));
	AddString(vol_slider_id, format("%f\\%+d", db2slider(5), (int)5));
	AddString(vol_slider_id, format("%f\\%d", db2slider(0), 0));
	AddString(vol_slider_id, format("%f\\%d", db2slider(-5), (int)-5));
	AddString(vol_slider_id, format("%f\\%d", db2slider(-10), (int)-10));
	AddString(vol_slider_id, format("%f\\%d", db2slider(-20), (int)-20));
	AddString(vol_slider_id, format("%f\\-inf", db2slider(DB_MIN))); // \u221e
	SetTooltip(vol_slider_id, _("Lautst&arke in dB"));
	AddCheckBox("Stumm", 0, 3, 0, 0, mute_id);

	EventM(vol_slider_id, this, &TrackMixer::onVolume);
	EventM(pan_slider_id, this, &TrackMixer::onPanning);
	EventM(mute_id, this, &TrackMixer::onMute);
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
	track->SetVolume(slider2vol(GetFloat("")));
}

void TrackMixer::onMute()
{
	track->SetMuted(IsChecked(""));
}

void TrackMixer::onPanning()
{
	track->SetPanning(GetFloat("")*2 - 1);
}

void TrackMixer::setTrack(Track* t)
{
	track = t;
	update();
}

void TrackMixer::update()
{
	SetFloat(vol_slider_id, vol2slider(track->volume));
	SetFloat(pan_slider_id, track->panning * 0.5f + 0.5f);
	Check(mute_id, track->muted);
	SetString(id_name, "!bold\\" + track->GetNiceName());
}


MixingConsole::MixingConsole(AudioFile *_audio, AudioOutput *_output, AudioStream *stream) :
	BottomBarConsole(_("Mischpult")),
	Observer("MixingConsole")
{
	audio = _audio;
	output = _output;
	id_inner = "inner_table";


	AddControlTable("", 0, 0, 1, 20, id_inner);
	SetTarget(id_inner, 0);
	AddControlTable("", 0, 0, 1, 5, "mc_output");
	AddSeparator("!vertical", 1, 0, 0, 0, "");


	SetTarget("mc_output", 0);
	AddText(_("!bold,center\\Ausgabe"), 0, 0, 0, 0, "");
	AddDrawingArea("!width=100,height=30,noexpandx,noexpandy", 0, 1, 0, 0, "mc_output_peaks");
	AddSlider("!vertical,expandy", 0, 2, 0, 0, "mc_output_volume");

	peak_meter = new PeakMeter(this, "mc_output_peaks", stream);
	SetFloat("mc_output_volume", output->getVolume());

	EventM("mc_output_volume", (HuiPanel*)this, (void(HuiPanel::*)())&MixingConsole::onOutputVolume);

	subscribe(audio);
	subscribe(output);
	loadData();
}

MixingConsole::~MixingConsole()
{
	unsubscribe(audio);
	unsubscribe(output);
	foreach(TrackMixer *m, mixer)
		delete(m);
	delete(peak_meter);
}

void MixingConsole::onOutputVolume()
{
	output->setVolume(GetFloat(""));
}

void MixingConsole::loadData()
{
	for (int i=mixer.num; i<audio->track.num; i++){
		TrackMixer *m = new TrackMixer();
		mixer.add(m);
		Embed(m, id_inner, i*2 + 2, 0);
		AddSeparator("!vertical", i*2 + 3, 0, 0, 0, "separator_" + i2s(i));
	}
	for (int i=audio->track.num; i<mixer.num; i++){
		delete(mixer[i]);
		RemoveControl("separator_" + i2s(i));
	}
	mixer.resize(audio->track.num);

	foreachi(Track *t, audio->track, i)
		mixer[i]->setTrack(t);
}

void MixingConsole::onUpdate(Observable* o, const string &message)
{
	if (o == output)
		SetFloat("mc_output_volume", output->getVolume());
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
