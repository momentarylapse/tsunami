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


class SingleMidiFxPanel : public HuiPanel, public Observer
{
public:
	SingleMidiFxPanel(Song *a, Track *t, MidiEffect *_fx, int _index) :
		Observer("SingleMidiFxPanel")
	{
		song = a;
		track = t;
		fx = _fx;
		index = _index;
		addGrid("!expandx,noexpandy", 0, 0, 1, 2, "grid");
		setTarget("grid", 0);
		addGrid("", 0, 0, 5, 1, "header");
		setTarget("header", 0);
		addButton("!flat", 0, 0, 0, 0, "load_favorite");
		setImage("load_favorite", "hui:open");
		setTooltip("load_favorite", _("Parameter laden"));
		addButton("!flat", 1, 0, 0, 0, "save_favorite");
		setImage("save_favorite", "hui:save");
		setTooltip("save_favorite", _("Parameter speichern"));
		addLabel("!bold,center,expandx\\" + fx->name, 2, 0, 0, 0, "");
		addCheckBox("", 3, 0, 0, 0, "enabled");
		setTooltip("enabled", _("aktiv?"));
		addButton("!flat", 4, 0, 0, 0, "delete");
		setImage("delete", "hui:delete");
		setTooltip("delete", _("Effekt l&oschen"));
		p = fx->createPanel();
		if (p){
			embed(p, "grid", 0, 1);
			p->update();
		}else{
			setTarget("grid", 0);
			addLabel(_("nicht konfigurierbar"), 0, 1, 0, 0, "");
			hideControl("load_favorite", true);
			hideControl("save_favorite", true);
		}

		event("enabled", this, &SingleMidiFxPanel::onEnabled);
		event("delete", this, &SingleMidiFxPanel::onDelete);
		event("load_favorite", this, &SingleMidiFxPanel::onLoad);
		event("save_favorite", this, &SingleMidiFxPanel::onSave);

		check("enabled", fx->enabled);

		old_param = fx->configToString();
		subscribe(fx, fx->MESSAGE_CHANGE);
		subscribe(fx, fx->MESSAGE_CHANGE_BY_ACTION);
	}
	virtual ~SingleMidiFxPanel()
	{
		unsubscribe(fx);
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
	virtual void onUpdate(Observable *o, const string &message)
	{
		if (message == o->MESSAGE_CHANGE){
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
	SideBarConsole(_("Midi Fx")),
	Observer("MidiFxConsole")
{
	view = _view;
	song = _song;

	fromResource("midi_fx_editor");

	id_inner = "midi_fx_inner_table";

	setTooltip("add", _("neuen Effekt hinzuf&ugen"));

	track = NULL;
	//Enable("add", false);
	enable("track_name", false);

	event("add", this, &MidiFxConsole::onAdd);

	event("edit_song", this, &MidiFxConsole::onEditSong);
	event("edit_track", this, &MidiFxConsole::onEditTrack);
	event("edit_midi", this, &MidiFxConsole::onEditMidi);

	subscribe(view, view->MESSAGE_CUR_TRACK_CHANGE);
	update();
}

MidiFxConsole::~MidiFxConsole()
{
	clear();
	unsubscribe(view);
	unsubscribe(song);
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

void MidiFxConsole::onUpdate(Observable* o, const string &message)
{
	update();
	if ((o == track) && (message == track->MESSAGE_DELETE)){
		setTrack(NULL);
	}else if ((o == view) && (message == view->MESSAGE_CUR_TRACK_CHANGE))
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
		unsubscribe(track);
	foreachi(HuiPanel *p, panels, i){
		delete(p);
		removeControl("separator_" + i2s(i));
	}
	panels.clear();
	track = NULL;
	//Enable("add", false);
}

void MidiFxConsole::onEditSong()
{
	((SideBar*)parent)->open(SideBar::SONG_CONSOLE);
}

void MidiFxConsole::onEditTrack()
{
	((SideBar*)parent)->open(SideBar::TRACK_CONSOLE);
}

void MidiFxConsole::onEditMidi()
{
	((SideBar*)parent)->open(SideBar::MIDI_EDITOR_CONSOLE);
}

void MidiFxConsole::setTrack(Track *t)
{
	clear();
	track = t;
	if (track){
		subscribe(track, track->MESSAGE_DELETE);
		subscribe(track, track->MESSAGE_ADD_MIDI_EFFECT);
		subscribe(track, track->MESSAGE_DELETE_MIDI_EFFECT);
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

