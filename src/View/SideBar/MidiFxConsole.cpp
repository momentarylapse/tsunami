/*
 * MidiFxConsole.cpp
 *
 *  Created on: 09.09.2014
 *      Author: michi
 */

#include "MidiFxConsole.h"
#include "../../Data/Track.h"
#include "../../Midi/MidiData.h"
#include "../../Audio/Synth/Synthesizer.h"
#include "../AudioView.h"
#include "../../Plugins/ConfigPanel.h"
#include "../../Plugins/MidiEffect.h"
#include "../../Plugins/PluginManager.h"
#include "../../Tsunami.h"


class SingleMidiFxPanel : public hui::Panel
{
public:
	SingleMidiFxPanel(Song *a, Track *t, MidiEffect *_fx, int _index)
	{
		song = a;
		track = t;
		fx = _fx;
		index = _index;

		fromResource("fx_panel");

		setString("name", fx->name);
		p = fx->createPanel();
		if (p){
			embed(p, "grid", 0, 1);
			p->update();
		}else{
			setTarget("grid", 0);
			addLabel(_("not configurable"), 0, 1, 0, 0, "");
			hideControl("load_favorite", true);
			hideControl("save_favorite", true);
		}

		event("enabled", std::bind(&SingleMidiFxPanel::onEnabled, this));
		event("delete", std::bind(&SingleMidiFxPanel::onDelete, this));
		event("load_favorite", std::bind(&SingleMidiFxPanel::onLoad, this));
		event("save_favorite", std::bind(&SingleMidiFxPanel::onSave, this));

		check("enabled", fx->enabled);

		old_param = fx->configToString();
		fx->subscribe_old2(this, SingleMidiFxPanel, fx->MESSAGE_CHANGE);
		fx->subscribe_old2(this, SingleMidiFxPanel, fx->MESSAGE_CHANGE_BY_ACTION);
	}
	virtual ~SingleMidiFxPanel()
	{
		fx->unsubscribe(this);
	}
	void onLoad()
	{
		string name = tsunami->plugin_manager->SelectFavoriteName(win, fx, false);
		if (name.num == 0)
			return;
		tsunami->plugin_manager->ApplyFavorite(fx, name);
		if (track)
			track->editMidiEffect(index, old_param);
		old_param = fx->configToString();
	}
	void onSave()
	{
		string name = tsunami->plugin_manager->SelectFavoriteName(win, fx, true);
		if (name.num == 0)
			return;
		tsunami->plugin_manager->SaveFavorite(fx, name);
	}
	void onEnabled()
	{
		if (track)
			track->enableMidiEffect(index, isChecked(""));
	}
	void onDelete()
	{
		if (track)
			track->deleteMidiEffect(index);
	}
	virtual void onUpdate(Observable *o)
	{
		if (o->cur_message() == o->MESSAGE_CHANGE){
			if (track)
				track->editMidiEffect(index, old_param);
		}
		check("enabled", fx->enabled);
		p->update();
		old_param = fx->configToString();
	}
	Song *song;
	Track *track;
	MidiEffect *fx;
	ConfigPanel *p;
	string old_param;
	int index;
};

MidiFxConsole::MidiFxConsole(AudioView *_view, Song *_song) :
	SideBarConsole(_("Midi Fx"))
{
	view = _view;
	song = _song;

	fromResource("midi_fx_editor");

	id_inner = "midi_fx_inner_table";

	track = NULL;
	//Enable("add", false);
	enable("track_name", false);

	event("add", std::bind(&MidiFxConsole::onAdd, this));

	event("edit_song", std::bind(&MidiFxConsole::onEditSong, this));
	event("edit_track", std::bind(&MidiFxConsole::onEditTrack, this));
	event("edit_midi", std::bind(&MidiFxConsole::onEditMidi, this));

	view->subscribe_old2(this, MidiFxConsole, view->MESSAGE_CUR_TRACK_CHANGE);
	update();
}

MidiFxConsole::~MidiFxConsole()
{
	clear();
	view->unsubscribe(this);
	song->unsubscribe(this);
}

void MidiFxConsole::update()
{
	bool allow = false;
	if (view->cur_track)
		allow = (view->cur_track->type == Track::TYPE_MIDI);
	hideControl("me_grid_yes", !allow);
	hideControl("me_grid_no", allow);
	hideControl(id_inner, !allow);
}

void MidiFxConsole::onUpdate(Observable* o)
{
	update();
	if ((o == track) and (o->cur_message() == track->MESSAGE_DELETE)){
		setTrack(NULL);
	}else if ((o == view) and (o->cur_message() == view->MESSAGE_CUR_TRACK_CHANGE))
		setTrack(view->cur_track);
	else
		setTrack(track);
}

void MidiFxConsole::onAdd()
{
	MidiEffect *effect = tsunami->plugin_manager->ChooseMidiEffect(this, track->song);
	if (!effect)
		return;
	if (track)
		track->addMidiEffect(effect);
}

void MidiFxConsole::clear()
{
	if (track)
		track->unsubscribe(this);
	foreachi(hui::Panel *p, panels, i){
		delete(p);
		removeControl("separator_" + i2s(i));
	}
	panels.clear();
	track = NULL;
	//Enable("add", false);
}

void MidiFxConsole::onEditSong()
{
	bar()->open(SideBar::SONG_CONSOLE);
}

void MidiFxConsole::onEditTrack()
{
	bar()->open(SideBar::TRACK_CONSOLE);
}

void MidiFxConsole::onEditMidi()
{
	bar()->open(SideBar::MIDI_EDITOR_CONSOLE);
}

void MidiFxConsole::setTrack(Track *t)
{
	clear();
	track = t;
	if (track){
		track->subscribe_old2(this, MidiFxConsole, track->MESSAGE_DELETE);
		track->subscribe_old2(this, MidiFxConsole, track->MESSAGE_ADD_MIDI_EFFECT);
		track->subscribe_old2(this, MidiFxConsole, track->MESSAGE_DELETE_MIDI_EFFECT);
	}


	if (track){
		foreachi(MidiEffect *e, track->midi.fx, i){
			panels.add(new SingleMidiFxPanel(song, track, e, i));
			embed(panels.back(), id_inner, 0, i*2 + 3);
			addSeparator("!horizontal", 0, i*2 + 4, 0, 0, "separator_" + i2s(i));
		}
		hideControl("comment_no_fx", track->midi.fx.num > 0);
	}else{
		hideControl("comment_no_fx", false);
	}
	//Enable("add", track);
}

