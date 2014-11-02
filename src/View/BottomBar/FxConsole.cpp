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
	SingleFxPanel(AudioFile *a, Track *t, Effect *_fx, int _index) :
		Observer("SingleFxPanel")
	{
		audio = a;
		track = t;
		fx = _fx;
		index = _index;
		addControlTable("!noexpandx,expandy", 0, 0, 1, 2, "grid");
		setTarget("grid", 0);
		addControlTable("", 0, 0, 5, 1, "header");
		setTarget("header", 0);
		addButton("!flat", 0, 0, 0, 0, "load_favorite");
		setImage("load_favorite", "hui:open");
		setTooltip("load_favorite", _("Parameter laden"));
		addButton("!flat", 1, 0, 0, 0, "save_favorite");
		setImage("save_favorite", "hui:save");
		setTooltip("save_favorite", _("Parameter speichern"));
		addText("!bold,center,expandx\\" + fx->name, 2, 0, 0, 0, "");
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
			addText(_("nicht konfigurierbar"), 0, 1, 0, 0, "");
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
			audio->editEffect(index, old_param);
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
			audio->enableEffect(index, isChecked(""));
	}
	void onDelete()
	{
		if (track)
			track->deleteEffect(index);
		else
			audio->deleteEffect(index);
	}
	virtual void onUpdate(Observable *o, const string &message)
	{
		if (message == o->MESSAGE_CHANGE){
			if (track)
				track->editEffect(index, old_param);
			else
				audio->editEffect(index, old_param);
		}
		check("enabled", fx->enabled);
		p->update();
		old_param = fx->configToString();
	}
	AudioFile *audio;
	Track *track;
	Effect *fx;
	string old_param;
	ConfigPanel *p;
	int index;
};

FxConsole::FxConsole(AudioView *_view, AudioFile *_audio) :
	BottomBarConsole(_("Effekte")),
	Observer("FxConsole")
{
	view = _view;
	audio = _audio;
	id_inner = "fx_inner_table";

	addControlTable("!expandy", 0, 0, 1, 32, id_inner);
	setTarget(id_inner, 0);
	addText("!angle=90\\...", 0, 0, 0, 0, "track_name");
	addSeparator("!vertical", 1, 0, 0, 0, "");
	addText(_("- hier sind (noch) keine Effekte aktiv -"), 30, 0, 0, 0, "comment_no_fx");
	addButton("!expandy,flat", 31, 0, 0, 0, "add");
	setImage("add", "hui:add");
	setTooltip("add", _("neuen Effekt hinzuf&ugen"));

	track = NULL;
	//Enable("add", false);
	enable("track_name", false);

	event("add", this, &FxConsole::onAdd);

	subscribe(view, view->MESSAGE_CUR_TRACK_CHANGE);
	subscribe(audio, audio->MESSAGE_ADD_EFFECT);
	subscribe(audio, audio->MESSAGE_DELETE_EFFECT);
}

FxConsole::~FxConsole()
{
	clear();
	unsubscribe(view);
	unsubscribe(audio);
}

void FxConsole::onAdd()
{
	Effect *effect = tsunami->plugin_manager->ChooseEffect(this);
	if (!effect)
		return;
	if (track)
		track->addEffect(effect);
	else
		audio->addEffect(effect);
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
		setString("track_name", format(_("!angle=90\\wirken auf die Spur '%s'"), track->getNiceName().c_str()));
	}else
		setString("track_name", _("!angle=90\\wirken auf die komplette Datei"));


	Array<Effect*> fx;
	if (track)
		fx = track->fx;
	else
		fx = audio->fx;
	foreachi(Effect *e, fx, i){
		panels.add(new SingleFxPanel(audio, track, e, i));
		embed(panels.back(), id_inner, i*2 + 2, 0);
		addSeparator("!vertical", i*2 + 3, 0, 0, 0, "separator_" + i2s(i));
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

