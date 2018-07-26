/*
 * TrackConsole.cpp
 *
 *  Created on: 26.03.2012
 *      Author: michi
 */

#include "../../Data/Track.h"
#include "../../Data/base.h"
#include "../Helper/Slider.h"
#include "../../Module/Synth/Synthesizer.h"
#include "../Dialog/ConfigurableSelectorDialog.h"
#include "../Dialog/TuningDialog.h"
#include "../../Plugins/PluginManager.h"
#include "../../Session.h"
#include "../AudioView.h"
#include "TrackConsole.h"

TrackConsole::TrackConsole(Session *session) :
	SideBarConsole(_("Track properties"), session)
{
	track = nullptr;
	setBorderWidth(5);
	fromResource("track_dialog");
	setDecimals(1);

	Array<Instrument> instruments = Instrument::enumerate();
	for (Instrument &i: instruments)
		setString("instrument", i.name());

	loadData();
	view->subscribe(this, std::bind(&TrackConsole::onViewCurTrackChange, this), view->MESSAGE_CUR_TRACK_CHANGE);

	event("name", std::bind(&TrackConsole::onName, this));
	event("volume", std::bind(&TrackConsole::onVolume, this));
	event("panning", std::bind(&TrackConsole::onPanning, this));
	event("instrument", std::bind(&TrackConsole::onInstrument, this));
	event("edit_tuning", std::bind(&TrackConsole::onEditTuning, this));
	event("select_synth", std::bind(&TrackConsole::onSelectSynth, this));

	event("edit_song", std::bind(&TrackConsole::onEditSong, this));
	event("edit_fx", std::bind(&TrackConsole::onEditFx, this));
	event("edit_curves", std::bind(&TrackConsole::onEditCurves, this));
	event("_edit_midi", std::bind(&TrackConsole::onEditMidi, this));
	event("_edit_midi_fx", std::bind(&TrackConsole::onEditMidiFx, this));
	event("_edit_synth", std::bind(&TrackConsole::onEditSynth, this));
}

TrackConsole::~TrackConsole()
{
	view->unsubscribe(this);
	if (track)
		track->unsubscribe(this);
}

void TrackConsole::loadData()
{
	enable("name", track);
	enable("volume", track);
	enable("panning", track);
	enable("instrument", track);
	hideControl("td_t_edit", !track);
	if (track){
		setString("name", track->name);
		setOptions("name", "placeholder=" + track->getNiceName());
		setFloat("volume", amplitude2db(track->volume));
		setFloat("panning", track->panning * 100.0f);
		enable("_edit_midi", track->type == SignalType::MIDI);
		enable("_edit_midi_fx", track->type == SignalType::MIDI);
		enable("_edit_synth", track->type != SignalType::AUDIO);
		enable("select_synth", track->type != SignalType::AUDIO);

		Array<Instrument> instruments = Instrument::enumerate();
		foreachi(Instrument &ii, instruments, i){
			if (track->instrument == ii)
				setInt("instrument", i);
		}

		string tuning = format(_("%d strings: "), track->instrument.string_pitch.num);
		for (int i=0; i<track->instrument.string_pitch.num; i++){
			if (i > 0)
				tuning += ", ";
			tuning += pitch_name(track->instrument.string_pitch[i]);
		}
		if (track->instrument.string_pitch.num == 0)
			tuning = _(" - no strings -");
		setString("tuning", tuning);

		setString("select_synth", track->synth->module_subtype);
	}else{
		hideControl("td_t_bars", true);
		setString("tuning", "");
	}
}

void TrackConsole::setTrack(Track *t)
{
	if (track)
		track->unsubscribe(this);
	track = t;
	loadData();
	if (track)
		track->subscribe(this, std::bind(&TrackConsole::onUpdate, this));
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
	Array<int> tuning;
	Array<Instrument> instruments = Instrument::enumerate();
	track->setInstrument(instruments[n]);
}

void TrackConsole::onEditTuning()
{
	TuningDialog *dlg = new TuningDialog(win, track);
	dlg->run();
	delete(dlg);
}

void TrackConsole::onSelectSynth()
{
	if (!track)
		return;
	string name = session->plugin_manager->ChooseModule(win, session, ModuleType::SYNTHESIZER, track->synth->module_subtype);
	if (name != "")
		track->setSynthesizer(CreateSynthesizer(session, name));
}

void TrackConsole::applyData()
{
}

void TrackConsole::onEditSong()
{
	bar()->open(SideBar::SONG_CONSOLE);
}

void TrackConsole::onEditFx()
{
	bar()->open(SideBar::FX_CONSOLE);
}

void TrackConsole::onEditCurves()
{
	bar()->open(SideBar::CURVE_CONSOLE);
}

void TrackConsole::onEditMidi()
{
	bar()->open(SideBar::MIDI_EDITOR_CONSOLE);
}

void TrackConsole::onEditMidiFx()
{
	bar()->open(SideBar::MIDI_FX_CONCOLE);
}

void TrackConsole::onEditSynth()
{
	bar()->open(SideBar::SYNTH_CONSOLE);
}

void TrackConsole::onViewCurTrackChange()
{
	setTrack(view->cur_track);
}

void TrackConsole::onUpdate()
{
	if (track->cur_message() == track->MESSAGE_DELETE){
		setTrack(nullptr);
	}else{
		loadData();
	}
}
