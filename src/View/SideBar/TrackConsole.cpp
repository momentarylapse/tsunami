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
	view->subscribe(this, [&]{ on_view_cur_track_change(); }, view->MESSAGE_CUR_TRACK_CHANGE);

	event("name", [&]{ on_name(); });
	event("volume", [&]{ on_volume(); });
	event("panning", [&]{ on_panning(); });
	event("instrument", [&]{ on_instrument(); });
	event("edit_tuning", [&]{ on_edit_tuning(); });
	event("select_synth", [&]{ on_select_synth(); });

	event("edit_song", [&]{ on_edit_song(); });
	event("edit_fx", [&]{ on_edit_fx(); });
	event("edit_curves", [&]{ on_edit_curves(); });
	event("edit_midi", [&]{ on_edit_midi(); });
	event("edit_midi_fx", [&]{ on_edit_midi_fx(); });
	event("_edit_synth", [&]{ on_edit_synth(); });
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
		track->subscribe(this, [&]{ on_update(); });
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
	session->set_mode("default/song");
}

void TrackConsole::on_edit_fx()
{
	session->set_mode("default/fx");
}

void TrackConsole::on_edit_curves()
{
	session->set_mode("curves");
}

void TrackConsole::on_edit_midi()
{
	session->set_mode("midi");
}

void TrackConsole::on_edit_midi_fx()
{
	session->set_mode("default/midi-fx");
}

void TrackConsole::on_edit_synth()
{
	session->set_mode("default/synth");
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
