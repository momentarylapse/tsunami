/*
 * MidiFxConsole.cpp
 *
 *  Created on: 09.09.2014
 *      Author: michi
 */

#include "MidiFxConsole.h"
#include "../../Data/base.h"
#include "../../Data/Song.h"
#include "../../Data/Track.h"
#include "../../Data/Midi/MidiData.h"
#include "../../Module/Synth/Synthesizer.h"
#include "../../Module/Midi/MidiEffect.h"
#include "../../Module/ConfigPanel.h"
#include "../AudioView.h"
#include "../../Plugins/PluginManager.h"
#include "../../Session.h"


class SingleMidiFxPanel : public hui::Panel
{
public:
	SingleMidiFxPanel(Session *_session, Track *t, MidiEffect *_fx, int _index)
	{
		session = _session;
		song = session->song;
		track = t;
		fx = _fx;
		index = _index;

		from_resource("fx_panel");

		set_string("name", fx->module_subtype);
		p = fx->create_panel();
		if (p){
			embed(p, "grid", 0, 1);
			p->update();
		}else{
			set_target("grid");
			add_label(_("not configurable"), 0, 1, "");
			hide_control("load_favorite", true);
			hide_control("save_favorite", true);
		}

		event("enabled", std::bind(&SingleMidiFxPanel::onEnabled, this));
		event("delete", std::bind(&SingleMidiFxPanel::onDelete, this));
		event("load_favorite", std::bind(&SingleMidiFxPanel::onLoad, this));
		event("save_favorite", std::bind(&SingleMidiFxPanel::onSave, this));

		check("enabled", fx->enabled);

		old_param = fx->config_to_string();
		fx->subscribe(this, std::bind(&SingleMidiFxPanel::onFxChange, this), fx->MESSAGE_CHANGE);
		fx->subscribe(this, std::bind(&SingleMidiFxPanel::onFxChangeByAction, this), fx->MESSAGE_CHANGE_BY_ACTION);
	}
	virtual ~SingleMidiFxPanel()
	{
		fx->unsubscribe(this);
	}
	void onLoad()
	{
		string name = session->plugin_manager->select_favorite_name(win, fx, false);
		if (name.num == 0)
			return;
		session->plugin_manager->apply_favorite(fx, name);
		if (track)
			track->edit_midi_effect(fx, old_param);
		old_param = fx->config_to_string();
	}
	void onSave()
	{
		string name = session->plugin_manager->select_favorite_name(win, fx, true);
		if (name.num == 0)
			return;
		session->plugin_manager->save_favorite(fx, name);
	}
	void onEnabled()
	{
		if (track)
			track->enable_midi_effect(fx, is_checked(""));
	}
	void onDelete()
	{
		if (track)
			track->delete_midi_effect(fx);
	}
	void onFxChange()
	{
		if (track)
			track->edit_midi_effect(fx, old_param);
		check("enabled", fx->enabled);
		p->update();
		old_param = fx->config_to_string();
	}
	void onFxChangeByAction()
	{
		check("enabled", fx->enabled);
		p->update();
		old_param = fx->config_to_string();
	}
	Session *session;
	Song *song;
	Track *track;
	MidiEffect *fx;
	ConfigPanel *p;
	string old_param;
	int index;
};

MidiFxConsole::MidiFxConsole(Session *session) :
	SideBarConsole(_("Midi Fx"), session)
{
	from_resource("midi_fx_editor");

	id_inner = "midi_fx_inner_table";

	track = nullptr;
	/*enable("track_name", false);

	event("add", std::bind(&MidiFxConsole::onAdd, this));

	event("edit_song", std::bind(&MidiFxConsole::onEditSong, this));
	event("edit_track", std::bind(&MidiFxConsole::onEditTrack, this));
	event("edit_midi", std::bind(&MidiFxConsole::onEditMidi, this));

	view->subscribe(this, std::bind(&MidiFxConsole::onViewCurTrackChange, this), view->MESSAGE_CUR_TRACK_CHANGE);
	update();*/
}

MidiFxConsole::~MidiFxConsole()
{
	/*clear();
	view->unsubscribe(this);
	song->unsubscribe(this);*/
}

void MidiFxConsole::update()
{
	bool allow = false;
	if (view->cur_track())
		allow = (view->cur_track()->type == SignalType::MIDI);
	hide_control("me_grid_yes", !allow);
	hide_control("me_grid_no", allow);
	hide_control(id_inner, !allow);
}

void MidiFxConsole::on_view_cur_track_change()
{
	set_track(view->cur_track());
}

void MidiFxConsole::on_track_delete()
{
	set_track(nullptr);
}

void MidiFxConsole::on_update()
{
	update();
}

void MidiFxConsole::on_add()
{
	string name = session->plugin_manager->choose_module(win, session, ModuleType::MIDI_EFFECT);
	MidiEffect *effect = CreateMidiEffect(session, name);
	if (track)
		track->add_midi_effect(effect);
}

void MidiFxConsole::clear()
{
	if (track)
		track->unsubscribe(this);
	foreachi(hui::Panel *p, panels, i){
		delete(p);
		remove_control("separator_" + i2s(i));
	}
	panels.clear();
	track = nullptr;
	//Enable("add", false);
}

void MidiFxConsole::on_edit_song()
{
	bar()->open(SideBar::SONG_CONSOLE);
}

void MidiFxConsole::on_edit_track()
{
	bar()->open(SideBar::TRACK_CONSOLE);
}

void MidiFxConsole::on_edit_midi()
{
	bar()->open(SideBar::MIDI_EDITOR_CONSOLE);
}

void MidiFxConsole::set_track(Track *t)
{
	clear();
	track = t;
	if (track){
		track->subscribe(this, std::bind(&MidiFxConsole::on_track_delete, this), track->MESSAGE_DELETE);
		track->subscribe(this, std::bind(&MidiFxConsole::on_update, this), track->MESSAGE_ADD_MIDI_EFFECT);
		track->subscribe(this, std::bind(&MidiFxConsole::on_update, this), track->MESSAGE_DELETE_MIDI_EFFECT);
	}


	if (track){
		foreachi(MidiEffect *e, track->midi_fx, i){
			panels.add(new SingleMidiFxPanel(session, track, e, i));
			embed(panels.back(), id_inner, 0, i*2 + 3);
			add_separator("!horizontal", 0, i*2 + 4, "separator_" + i2s(i));
		}
		hide_control("comment_no_fx", track->midi_fx.num > 0);
	}else{
		hide_control("comment_no_fx", false);
	}
	//Enable("add", track);
	update();
}

