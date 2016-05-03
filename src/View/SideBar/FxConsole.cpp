/*
 * FxConsole.cpp
 *
 *  Created on: 20.03.2014
 *      Author: michi
 */

#include "FxConsole.h"
#include "../AudioView.h"
#include "../../Data/Track.h"
#include "../../Plugins/ConfigPanel.h"
#include "../../Plugins/Effect.h"
#include "../../Plugins/PluginManager.h"
#include "../../Tsunami.h"

class SingleFxPanel : public HuiPanel, public Observer
{
public:
	SingleFxPanel(Song *a, Track *t, Effect *_fx, int _index) :
		Observer("SingleFxPanel")
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

		event("enabled", this, &SingleFxPanel::onEnabled);
		event("delete", this, &SingleFxPanel::onDelete);
		event("load_favorite", this, &SingleFxPanel::onLoad);
		event("save_favorite", this, &SingleFxPanel::onSave);

		check("enabled", fx->enabled);

		old_param = fx->configToString();
		subscribe(fx, fx->MESSAGE_CHANGE);
		subscribe(fx, fx->MESSAGE_CHANGE_BY_ACTION);
	}
	virtual ~SingleFxPanel()
	{
		unsubscribe(fx);
	}
	void onLoad()
	{
		string name = tsunami->plugin_manager->SelectFavoriteName(win, (Configurable*)fx, false);
		if (name.num == 0)
			return;
		tsunami->plugin_manager->ApplyFavorite(fx, name);
		if (track)
			track->editEffect(index, old_param);
		else
			song->editEffect(index, old_param);
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
			track->enableEffect(index, isChecked(""));
		else
			song->enableEffect(index, isChecked(""));
	}
	void onDelete()
	{
		if (track)
			track->deleteEffect(index);
		else
			song->deleteEffect(index);
	}
	virtual void onUpdate(Observable *o, const string &message)
	{
		if (message == o->MESSAGE_CHANGE){
			if (track)
				track->editEffect(index, old_param);
			else
				song->editEffect(index, old_param);
		}
		check("enabled", fx->enabled);
		p->update();
		old_param = fx->configToString();
	}
	Song *song;
	Track *track;
	Effect *fx;
	string old_param;
	ConfigPanel *p;
	int index;
};

FxConsole::FxConsole(AudioView *_view, Song *_song) :
	SideBarConsole(_("Effekte")),
	Observer("FxConsole")
{
	view = _view;
	song = _song;
	id_inner = "fx_inner_table";

	fromResource("fx_editor");

	track = NULL;
	//Enable("add", false);

	if (!view)
		hideControl("edit_track", true);

	event("add", this, &FxConsole::onAdd);

	event("edit_song", this, &FxConsole::onEditSong);
	event("edit_track", this, &FxConsole::onEditTrack);

	if (view)
		subscribe(view, view->MESSAGE_CUR_TRACK_CHANGE);
	subscribe(song, song->MESSAGE_NEW);
	subscribe(song, song->MESSAGE_ADD_EFFECT);
	subscribe(song, song->MESSAGE_DELETE_EFFECT);
}

FxConsole::~FxConsole()
{
	clear();
	if (view)
		unsubscribe(view);
	unsubscribe(song);
}

void FxConsole::onAdd()
{
	Effect *effect = tsunami->plugin_manager->ChooseEffect(this, track->song);
	if (!effect)
		return;
	if (track)
		track->addEffect(effect);
	else
		song->addEffect(effect);
}

void FxConsole::onEditSong()
{
	((SideBar*)parent)->open(SideBar::SONG_CONSOLE);
}

void FxConsole::onEditTrack()
{
	((SideBar*)parent)->open(SideBar::TRACK_CONSOLE);
}

void FxConsole::clear()
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

void FxConsole::setTrack(Track *t)
{
	clear();
	track = t;
	if (track){
		subscribe(track, track->MESSAGE_DELETE);
		subscribe(track, track->MESSAGE_ADD_EFFECT);
		subscribe(track, track->MESSAGE_DELETE_EFFECT);
	}


	Array<Effect*> fx;
	if (track)
		fx = track->fx;
	else
		fx = song->fx;
	foreachi(Effect *e, fx, i){
		panels.add(new SingleFxPanel(song, track, e, i));
		embed(panels.back(), id_inner, 0, i*2);
		addSeparator("!horizontal", 0, i*2 + 1, 0, 0, "separator_" + i2s(i));
	}
	hideControl("comment_no_fx", fx.num > 0);
	//Enable("add", track);
}

void FxConsole::onUpdate(Observable* o, const string &message)
{
	if ((o == track) && (message == track->MESSAGE_DELETE)){
		setTrack(NULL);
	}else if ((o == view) && (message == view->MESSAGE_CUR_TRACK_CHANGE))
		setTrack(view->cur_track);
	else
		setTrack(track);
}

