/*
 * TrackConsole.cpp
 *
 *  Created on: 26.03.2012
 *      Author: michi
 */

#include "../../Data/Track.h"
#include "../Helper/Slider.h"
#include "../../Audio/Synth/Synthesizer.h"
#include "../Dialog/ConfigurableSelectorDialog.h"
#include "../../Plugins/PluginManager.h"
#include "../AudioView.h"
#include "TrackConsole.h"

TrackConsole::TrackConsole(AudioView *_view) :
	SideBarConsole(_("Spur Eigenschaften")),
	Observer("TrackConsole")
{
	view = _view;
	track = NULL;
	setBorderWidth(5);
	fromResource("track_dialog");
	setDecimals(1);

	setString("instrument", _("    - keins -"));
	Array<string> instruments = get_instruments();
	foreach(string &i, instruments)
		setString("instrument", get_instrument_name(i));

	num_strings = 0;

	loadData();
	subscribe(view, view->MESSAGE_CUR_TRACK_CHANGE);

	event("name", this, &TrackConsole::onName);
	event("volume", this, &TrackConsole::onVolume);
	event("panning", this, &TrackConsole::onPanning);
	event("instrument", this, &TrackConsole::onInstrument);
	event("strings", this, &TrackConsole::onStrings);
	for (int i=0; i<100; i++)
		event(format("string%d", i), this, &TrackConsole::onString);

	event("edit_song", this, &TrackConsole::onEditSong);
	event("edit_fx", this, &TrackConsole::onEditFx);
	event("edit_curves", this, &TrackConsole::onEditCurves);
	event("edit_midi", this, &TrackConsole::onEditMidi);
	event("edit_midi_fx", this, &TrackConsole::onEditMidiFx);
	event("edit_synth", this, &TrackConsole::onEditSynth);
	event("edit_bars", this, &TrackConsole::onEditBars);
}

TrackConsole::~TrackConsole()
{
	unsubscribe(view);
	if (track)
		unsubscribe(track);
}

void TrackConsole::loadData()
{
	enable("name", track);
	enable("volume", track);
	enable("panning", track);
	enable("instrument", track);
	enable("strings", track);
	hideControl("td_t_edit", !track);
	if (track){
		setString("name", track->name);
		setOptions("name", "placeholder=" + track->getNiceName());
		setFloat("volume", amplitude2db(track->volume));
		setFloat("panning", track->panning * 100.0f);
		enable("edit_midi", track->type == Track::TYPE_MIDI);
		enable("edit_midi_fx", track->type == Track::TYPE_MIDI);
		enable("edit_synth", track->type != Track::TYPE_AUDIO);

		setInt("instrument", 0);
		Array<string> instruments = get_instruments();
		foreachi(string &ii, instruments, i){
			if (track->instrument == ii)
				setInt("instrument", i + 1);
		}
		for (int i=track->tuning.num; i<num_strings; i++){
			removeControl(format("string%d", i));
			removeControl(format("string%d_label", i));
		}
		setInt("strings", track->tuning.num);
		setTarget("td_g_tuning", 0);
		foreachi(int t, track->tuning, i){
			string id = format("string%d", i);
			if (i >= num_strings){
				addLabel(i2s(i+1), 0, 100 - i, 0, 0, format("string%d_label", i));
				addComboBox("", 1, 100 - i, 0, 0, id);
				for (int p=0; p<128; p++)
					setString(id, pitch_name(p));
			}
			setInt(id, t);
		}
		num_strings = track->tuning.num;
	}else{
		hideControl("td_t_bars", true);
	}
}

void TrackConsole::setTrack(Track *t)
{
	if (track)
		unsubscribe(track);
	track = t;
	loadData();
	if (track)
		subscribe(track);
}

void TrackConsole::onName()
{
	track->setName(getString(""));
}

void TrackConsole::onVolume()
{
	track->setVolume(db2amplitude(getFloat("volume")));
}

void TrackConsole::onPanning()
{
	track->setPanning(getFloat("panning") / 100.0f);
}

void TrackConsole::onInstrument()
{
	int n = getInt("");
	string instrument;
	Array<int> tuning;
	Array<string> instruments = get_instruments();
	if (n > 0){
		instrument = instruments[n - 1];
		tuning = get_default_tuning(instrument);
	}
	track->setInstrument(instrument, tuning);
}

void TrackConsole::applyData()
{
}

void TrackConsole::onEditSong()
{
	((SideBar*)parent)->open(SideBar::SONG_CONSOLE);
}

void TrackConsole::onEditFx()
{
	((SideBar*)parent)->open(SideBar::FX_CONSOLE);
}

void TrackConsole::onEditCurves()
{
	((SideBar*)parent)->open(SideBar::CURVE_CONSOLE);
}

void TrackConsole::onEditMidi()
{
	((SideBar*)parent)->open(SideBar::MIDI_EDITOR);
}

void TrackConsole::onEditMidiFx()
{
	((SideBar*)parent)->open(SideBar::MIDI_FX_CONCOLE);
}

void TrackConsole::onEditSynth()
{
	((SideBar*)parent)->open(SideBar::SYNTH_CONSOLE);
}

void TrackConsole::onEditBars()
{
	((SideBar*)parent)->open(SideBar::BARS_CONSOLE);
}

void TrackConsole::onUpdate(Observable *o, const string &message)
{
	if (o == view){
		setTrack(view->cur_track);
	}else if ((o == track) and (message == track->MESSAGE_DELETE)){
		setTrack(NULL);
	}else{
		loadData();
	}
}

void TrackConsole::onStrings()
{
	int n = getInt("");
	Array<int> tuning = track->tuning;
	tuning.resize(n);
	track->setInstrument(track->instrument, tuning);
}

void TrackConsole::onString()
{
	string id = HuiGetEvent()->id;
	int n = id.substr(6, -1)._int();
	int p = getInt(id);
	Array<int> tuning = track->tuning;
	tuning[n] = p;
	track->setInstrument(track->instrument, tuning);
}
