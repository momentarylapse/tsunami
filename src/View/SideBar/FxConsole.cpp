/*
 * FxConsole.cpp
 *
 *  Created on: 20.03.2014
 *      Author: michi
 */

#include "FxConsole.h"
#include "../AudioView.h"
#include "../../Data/Song.h"
#include "../../Data/Track.h"
#include "../../Module/Audio/AudioEffect.h"
#include "../../Module/ConfigPanel.h"
#include "../../Plugins/PluginManager.h"
#include "../../Session.h"

class SingleFxPanel : public hui::Panel
{
public:
	SingleFxPanel(Session *_session, FxConsole *_console, Track *t, AudioEffect *_fx, int _index)
	{
		session = _session;
		console = _console;
		song = session->song;
		track = t;
		fx = _fx;
		index = _index;

		fromResource("fx_panel");

		setString("name", fx->module_subtype);

		p = fx->create_panel();
		if (p){
			embed(p, "content", 0, 0);
			p->update();
		}else{
			setTarget("content");
			addLabel(_("not configurable"), 0, 1, "");
			hideControl("load_favorite", true);
			hideControl("save_favorite", true);
		}

		event("enabled", std::bind(&SingleFxPanel::onEnabled, this));
		event("delete", std::bind(&SingleFxPanel::onDelete, this));
		event("load_favorite", std::bind(&SingleFxPanel::onLoad, this));
		event("save_favorite", std::bind(&SingleFxPanel::onSave, this));
		event("show_large", std::bind(&SingleFxPanel::on_large, this));

		check("enabled", fx->enabled);

		old_param = fx->config_to_string();
		fx->subscribe(this, std::bind(&SingleFxPanel::onfxChange, this), fx->MESSAGE_CHANGE);
		fx->subscribe(this, std::bind(&SingleFxPanel::onfxChangeByAction, this), fx->MESSAGE_CHANGE_BY_ACTION);
	}
	virtual ~SingleFxPanel()
	{
		fx->unsubscribe(this);
	}
	void onLoad()
	{
		string name = session->plugin_manager->SelectFavoriteName(win, fx, false);
		if (name.num == 0)
			return;
		session->plugin_manager->ApplyFavorite(fx, name);
		if (track)
			track->editEffect(fx, old_param);
		else
			song->editEffect(fx, old_param);
		old_param = fx->config_to_string();
	}
	void onSave()
	{
		string name = session->plugin_manager->SelectFavoriteName(win, fx, true);
		if (name.num == 0)
			return;
		session->plugin_manager->SaveFavorite(fx, name);
	}
	void onEnabled()
	{
		if (track)
			track->enableEffect(fx, isChecked(""));
		else
			song->enableEffect(fx, isChecked(""));
	}
	void onDelete()
	{
		hui::RunLater(0, [&]{
			if (track)
				track->deleteEffect(fx);
			else
				song->deleteEffect(fx);
		});
	}
	void on_large()
	{
		console->set_exclusive(fx);

	}
	void onfxChange()
	{
		if (track)
			track->editEffect(fx, old_param);
		else
			song->editEffect(fx, old_param);
		check("enabled", fx->enabled);
		p->update();
		old_param = fx->config_to_string();

	}
	void onfxChangeByAction()
	{
		check("enabled", fx->enabled);
		p->update();
		old_param = fx->config_to_string();
	}
	Session *session;
	Song *song;
	Track *track;
	AudioEffect *fx;
	string old_param;
	ConfigPanel *p;
	int index;
	FxConsole *console;
};

FxConsole::FxConsole(Session *session) :
	SideBarConsole(_("Effects"), session)
{
	id_inner = "fx_inner_table";

	fromResource("fx_editor");

	track = nullptr;
	exclusive = nullptr;
	//Enable("add", false);

	if (!view)
		hideControl("edit_track", true);

	event("add", std::bind(&FxConsole::onAdd, this));

	event("edit_song", std::bind(&FxConsole::onEditSong, this));
	event("edit_track", std::bind(&FxConsole::onEditTrack, this));

	if (view)
		view->subscribe(this, std::bind(&FxConsole::onViewCurTrackChange, this), view->MESSAGE_CUR_TRACK_CHANGE);
	song->subscribe(this, std::bind(&FxConsole::onUpdate, this), song->MESSAGE_NEW);
	song->subscribe(this, std::bind(&FxConsole::onUpdate, this), song->MESSAGE_ADD_EFFECT);
	song->subscribe(this, std::bind(&FxConsole::onUpdate, this), song->MESSAGE_DELETE_EFFECT);
}

FxConsole::~FxConsole()
{
	clear();
	if (view)
		view->unsubscribe(this);
	song->unsubscribe(this);
}

void FxConsole::onAdd()
{
	string name = session->plugin_manager->ChooseModule(win, session, ModuleType::AUDIO_EFFECT);
	if (name == "")
		return;
	AudioEffect *effect = CreateAudioEffect(session, name);
	if (track)
		track->addEffect(effect);
	else
		song->addEffect(effect);
}

void FxConsole::onEditSong()
{
	bar()->open(SideBar::SONG_CONSOLE);
}

void FxConsole::onEditTrack()
{
	bar()->open(SideBar::TRACK_CONSOLE);
}

void FxConsole::clear()
{
	if (track)
		track->unsubscribe(this);
	foreachi(hui::Panel *p, panels, i){
		delete(p);
		removeControl("separator_" + i2s(i));
	}
	panels.clear();
	track = nullptr;
	//Enable("add", false);
}

void FxConsole::setTrack(Track *t)
{
	clear();
	set_exclusive(nullptr);
	track = t;
	if (track){
		track->subscribe(this, std::bind(&FxConsole::onTrackDelete, this), track->MESSAGE_DELETE);
		track->subscribe(this, std::bind(&FxConsole::onUpdate, this), track->MESSAGE_ADD_EFFECT);
		track->subscribe(this, std::bind(&FxConsole::onUpdate, this), track->MESSAGE_DELETE_EFFECT);
	}


	Array<AudioEffect*> fx;
	if (track)
		fx = track->fx;
	else
		fx = song->fx;
	foreachi(AudioEffect *e, fx, i){
		auto *p = new SingleFxPanel(session, this, track, e, i);
		p->hideControl("content", !allow_show(e));
		panels.add(p);
		embed(panels.back(), id_inner, 0, i*2);
		addSeparator("!horizontal", 0, i*2 + 1, "separator_" + i2s(i));
	}
	hideControl("comment_no_fx", fx.num > 0);
	//Enable("add", track);
}

void FxConsole::set_exclusive(AudioEffect *fx)
{
	exclusive = fx;
	bar()->set_large(exclusive);
	for (auto *p: panels){
		SingleFxPanel *pp = (SingleFxPanel*)p;
		pp->hideControl("content", !allow_show(pp->fx));
		pp->p->set_large(exclusive == pp->fx);
	}
}

bool FxConsole::allow_show(AudioEffect *fx)
{
	if (exclusive)
		return fx == exclusive;
	return true;
}

void FxConsole::onTrackDelete()
{
	setTrack(nullptr);
}
void FxConsole::onViewCurTrackChange()
{
	setTrack(view->cur_track());
}

void FxConsole::onUpdate()
{
	setTrack(track);
}

