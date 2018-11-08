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
	editing = false;
	set_border_width(5);
	from_resource("track_dialog");
	set_decimals(1);

	Array<Instrument> instruments = Instrument::enumerate();
	for (Instrument &i: instruments)
		set_string("instrument", i.name());

	load_data();
	view->subscribe(this, std::bind(&TrackConsole::on_view_cur_track_change, this), view->MESSAGE_CUR_TRACK_CHANGE);

	event("name", std::bind(&TrackConsole::on_name, this));
	event("volume", std::bind(&TrackConsole::on_volume, this));
	event("panning", std::bind(&TrackConsole::on_panning, this));
	event("instrument", std::bind(&TrackConsole::on_instrument, this));
	event("edit_tuning", std::bind(&TrackConsole::on_edit_tuning, this));
	event("select_synth", std::bind(&TrackConsole::on_select_synth, this));

	event("edit_song", std::bind(&TrackConsole::on_edit_song, this));
	event("edit_fx", std::bind(&TrackConsole::on_edit_fx, this));
	event("edit_curves", std::bind(&TrackConsole::on_edit_curves, this));
	event("edit_midi", std::bind(&TrackConsole::on_edit_midi, this));
	event("edit_midi_fx", std::bind(&TrackConsole::on_edit_midi_fx, this));
	event("_edit_synth", std::bind(&TrackConsole::on_edit_synth, this));
}

TrackConsole::~TrackConsole()
{
	view->unsubscribe(this);
	if (track)
		track->unsubscribe(this);
}

void TrackConsole::load_data()
{
	enable("name", track);
	enable("volume", track);
	enable("panning", track);
	enable("instrument", track);
	hide_control("td_t_edit", !track);
	if (track){
		set_string("name", track->name);
		set_options("name", "placeholder=" + track->nice_name());
		set_float("volume", amplitude2db(track->volume));
		set_float("panning", track->panning * 100.0f);
		hide_control("edit_midi", track->type != SignalType::MIDI);
		hide_control("edit_midi_fx", track->type != SignalType::MIDI);
		enable("_edit_synth", track->type != SignalType::AUDIO);
		enable("select_synth", track->type != SignalType::AUDIO);

		Array<Instrument> instruments = Instrument::enumerate();
		foreachi(Instrument &ii, instruments, i){
			if (track->instrument == ii)
				set_int("instrument", i);
		}

		update_strings();

		set_string("select_synth", track->synth->module_subtype);
	}else{
		hide_control("td_t_bars", true);
		set_string("tuning", "");
	}
}

void TrackConsole::update_strings()
{
	string tuning = format(_("%d strings: "), track->instrument.string_pitch.num);
	for (int i=0; i<track->instrument.string_pitch.num; i++){
		if (i > 0)
			tuning += ", ";
		tuning += pitch_name(track->instrument.string_pitch[i]);
	}
	if (track->instrument.string_pitch.num == 0)
		tuning = _(" - no strings -");
	set_string("tuning", tuning);
}

void TrackConsole::set_track(Track *t)
{
	if (track)
		track->unsubscribe(this);
	track = t;
	load_data();
	if (track)
		track->subscribe(this, std::bind(&TrackConsole::on_update, this));
}

void TrackConsole::on_name()
{
	editing = true;
	track->set_name(get_string(""));
	editing = false;
}

void TrackConsole::on_volume()
{
	editing = true;
	track->set_volume(db2amplitude(get_float("volume")));
	editing = false;
}

void TrackConsole::on_panning()
{
	editing = true;
	track->set_panning(get_float("panning") / 100.0f);
	editing = false;
}

void TrackConsole::on_instrument()
{
	editing = true;
	int n = get_int("");
	Array<int> tuning;
	Array<Instrument> instruments = Instrument::enumerate();
	track->set_instrument(instruments[n]);
	update_strings();
	editing = false;
}

void TrackConsole::on_edit_tuning()
{
	TuningDialog *dlg = new TuningDialog(win, track);
	dlg->run();
	delete(dlg);
}

void TrackConsole::on_select_synth()
{
	if (!track)
		return;
	string name = session->plugin_manager->choose_module(win, session, ModuleType::SYNTHESIZER, track->synth->module_subtype);
	if (name != "")
		track->set_synthesizer(CreateSynthesizer(session, name));
}

void TrackConsole::on_edit_song()
{
	bar()->open(SideBar::SONG_CONSOLE);
}

void TrackConsole::on_edit_fx()
{
	bar()->open(SideBar::FX_CONSOLE);
}

void TrackConsole::on_edit_curves()
{
	bar()->open(SideBar::CURVE_CONSOLE);
}

void TrackConsole::on_edit_midi()
{
	bar()->open(SideBar::MIDI_EDITOR_CONSOLE);
}

void TrackConsole::on_edit_midi_fx()
{
	bar()->open(SideBar::MIDI_FX_CONCOLE);
}

void TrackConsole::on_edit_synth()
{
	bar()->open(SideBar::SYNTH_CONSOLE);
}

void TrackConsole::on_view_cur_track_change()
{
	set_track(view->cur_track());
}

void TrackConsole::on_update()
{
	if (track->cur_message() == track->MESSAGE_DELETE){
		set_track(nullptr);
	}else{
		if (!editing)
			load_data();
	}
}
