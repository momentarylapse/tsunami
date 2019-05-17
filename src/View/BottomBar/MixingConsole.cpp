/*
 * MixingConsole.cpp
 *
 *  Created on: 16.03.2014
 *      Author: michi
 */

#include "MixingConsole.h"
#include "../Helper/PeakMeterDisplay.h"
#include <math.h>

#include "../../Data/base.h"
#include "../../Data/Song.h"
#include "../../Data/Track.h"
#include "../../Module/Audio/AudioEffect.h"
#include "../../Module/ConfigPanel.h"
#include "../../Session.h"
#include "../../Plugins/PluginManager.h"
#include "../../Device/DeviceManager.h"
#include "../../Stream/AudioOutput.h"
#include "../AudioView.h"
#include "../AudioViewTrack.h"

//string module_header(Module *m);

extern const int CONFIG_PANEL_WIDTH;

class TrackMixer: public hui::Panel
{
public:
	TrackMixer(AudioViewTrack *t, MixingConsole *c)
	{
		set_border_width(0);
		from_resource("track-mixer2");

		track = nullptr;
		vtrack = nullptr;
		editing = false;
		console = c;

		reveal("revealer-volume", true);
		hide_control("grid-fx", true);

		id_separator = "mixing-track-separator";
		id_name = "name";
		vol_slider_id = "volume";
		pan_slider_id = "panning";
		mute_id = "mute";
		add_string(pan_slider_id, "-1\\L");
		add_string(pan_slider_id, "0\\");
		add_string(pan_slider_id, "1\\R");
		add_string(vol_slider_id, format("%f\\%+d", db2slider(DB_MAX), (int)DB_MAX));
		add_string(vol_slider_id, format("%f\\%+d", db2slider(5), (int)5));
		add_string(vol_slider_id, format("%f\\%d", db2slider(0), 0));
		add_string(vol_slider_id, format("%f\\%d", db2slider(-5), (int)-5));
		add_string(vol_slider_id, format("%f\\%d", db2slider(-10), (int)-10));
		add_string(vol_slider_id, format("%f\\%d", db2slider(-20), (int)-20));
		add_string(vol_slider_id, format("%f\\-\u221e", db2slider(DB_MIN))); // \u221e

		event(vol_slider_id, [=]{ on_volume(); });
		event(pan_slider_id, [=]{ on_panning(); });
		event(mute_id, [=]{ on_mute(); });
		event("solo", [=]{ on_solo(); });
		event_x("fx", "hui:select", [=]{ on_fx_select(); });
		event_x("fx", "hui:change", [=]{ on_fx_edit(); });
		event_x("fx", "hui:move", [=]{ on_fx_move(); });
		event("add-fx", [=]{ on_add_fx(); });

		vtrack = t;
		track = t->track;
		vtrack->subscribe(this, [=]{ update(); }, vtrack->MESSAGE_CHANGE);
		vtrack->subscribe(this, [=]{ on_vtrack_delete(); }, vtrack->MESSAGE_DELETE);
		update();
	}
	~TrackMixer()
	{
		clear_track();
	}

	void on_volume()
	{
		editing = true;
		if (track){
			if (parent->is_checked("link-volumes"))
				track->song->change_all_track_volumes(track, slider2vol(get_float("")));
			else
				vtrack->set_volume(slider2vol(get_float("")));
		}
		editing = false;
	}

	// allow update() for mute/solo!
	void on_mute()
	{
		if (vtrack)
			vtrack->set_muted(is_checked(""));
	}
	void on_solo()
	{
		if (vtrack)
			vtrack->set_solo(is_checked(""));
	}

	void on_panning()
	{
		editing = true;
		if (vtrack)
			vtrack->set_panning(get_float(""));
		editing = false;
	}
	void clear_track()
	{
		if (vtrack)
			vtrack->unsubscribe(this);
		vtrack = nullptr;
		track = nullptr;
	}
	void on_vtrack_delete()
	{
		clear_track();

	}
	void set_mode(MixerMode mode)
	{
		hide_control("grid-volume", mode != MixerMode::VOLUME);
		hide_control("grid-fx", mode != MixerMode::EFFECTS);
		reveal("revealer-volume", mode == MixerMode::VOLUME);
		reveal("revealer-fx", mode == MixerMode::EFFECTS);
	}
	void on_fx_select()
	{
		int n = get_int("");
		if (n >= 0)
			console->select_module(track->fx[n]);
		else
			console->select_module(nullptr);
	}
	void on_fx_edit()
	{
		int n = hui::GetEvent()->row;
		if (n >= 0)
			track->enable_effect(track->fx[n], get_cell("", n, hui::GetEvent()->column)._bool());
	}
	void on_fx_move()
	{
		int s = hui::GetEvent()->row;
		int t = hui::GetEvent()->row_target;
		track->move_effect(s, t);
	}
	void on_add_fx()
	{
		string name = console->session->plugin_manager->choose_module(win, console->session, ModuleType::AUDIO_EFFECT);
		if (name == "")
			return;
		AudioEffect *effect = CreateAudioEffect(console->session, name);
		track->add_effect(effect);

	}
	void update()
	{
		if (!vtrack)
			return;
		if (editing)
			return;
		set_float(vol_slider_id, vol2slider(track->volume));
		set_float(pan_slider_id, track->panning);
		check(mute_id, track->muted);
		check("solo", vtrack->solo);
		bool is_playable = vtrack->view->get_playable_tracks().contains(track);
		enable(id_name, is_playable);
		if (is_playable)
			set_string(id_name, track->nice_name());
		else
			set_string(id_name, "<s>" + track->nice_name() + "</s>");

		// fx list
		reset("fx");
		for (auto *fx: vtrack->track->fx)
			add_string("fx", b2s(fx->enabled) + "\\" + fx->module_subtype);
		set_int("fx", vtrack->track->fx.find((AudioEffect*)console->selected_module));
	}


	static constexpr float DB_MIN = -1000000;
	static constexpr float DB_MAX = 10;
	static constexpr float TAN_SCALE = 10.0f;

	static float db2slider(float db)
	{
		return (atan(db / TAN_SCALE) - atan(DB_MIN / TAN_SCALE)) / (atan(DB_MAX / TAN_SCALE) - atan(DB_MIN / TAN_SCALE));
	}
	static float slider2db(float val)
	{
		return tan(atan(DB_MIN / TAN_SCALE) + val * (atan(DB_MAX / TAN_SCALE)- atan(DB_MIN / TAN_SCALE))) * TAN_SCALE;
	}
	static float vol2slider(float vol)
	{
		return db2slider(amplitude2db(vol));
	}
	static float slider2vol(float val)
	{
		return db2amplitude(slider2db(val));
	}


	Track *track;
	AudioViewTrack *vtrack;
	//Slider *volume_slider;
	//Slider *panning_slider;
	string id_name;
	string vol_slider_id;
	string pan_slider_id;
	string mute_id;
	string id_separator;
	AudioView *view;
	bool editing;
	MixingConsole *console;
};




class FxPanel : public hui::Panel
{
public:
	FxPanel(Session *_session, MixingConsole *_console, AudioEffect *_fx, std::function<void(bool)> _func_enable, std::function<void()> _func_delete, std::function<void(const string&)> _func_edit)
	{
		session = _session;
		console = _console;
		fx = _fx;

		func_enable = _func_enable;
		func_delete = _func_delete;
		func_edit = _func_edit;

		from_resource("fx_panel");

		set_string("name", fx->module_subtype);

		p = fx->create_panel();
		if (p){
			embed(p, "content", 0, 0);
			p->update();
		}else{
			set_target("content");
			add_label(_("not configurable"), 0, 1, "");
			hide_control("load_favorite", true);
			hide_control("save_favorite", true);
		}
		set_options("content", format("width=%d", CONFIG_PANEL_WIDTH));

		event("enabled", [=]{ on_enabled(); });
		event("delete", [=]{ on_delete(); });
		event("load_favorite", [=]{ on_load(); });
		event("save_favorite", [=]{ on_save(); });
		event("show_large", [=]{ on_large(); });

		check("enabled", fx->enabled);

		old_param = fx->config_to_string();
		fx->subscribe(this, [=]{ on_fx_change(); }, fx->MESSAGE_CHANGE);
		fx->subscribe(this, [=]{ on_fx_change_by_action(); }, fx->MESSAGE_CHANGE_BY_ACTION);
	}
	virtual ~FxPanel()
	{
		fx->unsubscribe(this);
	}
	void on_load()
	{
		string name = session->plugin_manager->select_favorite_name(win, fx, false);
		if (name.num == 0)
			return;
		session->plugin_manager->apply_favorite(fx, name);
		func_edit(old_param);
		old_param = fx->config_to_string();
	}
	void on_save()
	{
		string name = session->plugin_manager->select_favorite_name(win, fx, true);
		if (name.num == 0)
			return;
		session->plugin_manager->save_favorite(fx, name);
	}
	void on_enabled()
	{
		func_enable(is_checked(""));
	}
	void on_delete()
	{
		hui::RunLater(0, func_delete);
	}
	void on_large()
	{
		//console->set_exclusive(this);
		p->set_large(true);

	}
	void on_fx_change()
	{
		func_edit(old_param);
		check("enabled", fx->enabled);
		if (p)
			p->update();
		old_param = fx->config_to_string();

	}
	void on_fx_change_by_action()
	{
		check("enabled", fx->enabled);
		if (p)
			p->update();
		old_param = fx->config_to_string();
	}
	std::function<void(bool)> func_enable;
	std::function<void(const string&)> func_edit;
	std::function<void()> func_delete;
	Session *session;
	AudioEffect *fx;
	string old_param;
	ConfigPanel *p;
	MixingConsole *console;
};



MixingConsole::MixingConsole(Session *session) :
	BottomBar::Console(_("Mixer"), session)
{
	device_manager = session->device_manager;
	id_inner = "inner-grid";

	set_border_width(2);
	from_resource("mixing-console");

	config_panel = nullptr;
	selected_module = nullptr;
	select_module(nullptr);

	peak_meter = new PeakMeterDisplay(this, "output-peaks", view->peak_meter);
	set_float("output-volume", device_manager->get_output_volume());

	event("output-volume", [=]{ on_output_volume(); });
	event("show-vol", [=]{ set_mode(MixerMode::VOLUME); });
	event("show-fx", [=]{ set_mode(MixerMode::EFFECTS); });

	view->subscribe(this, [=]{ on_tracks_change(); }, session->view->MESSAGE_VTRACK_CHANGE);
	view->subscribe(this, [=]{ update_all(); }, session->view->MESSAGE_SOLO_CHANGE);
	song->subscribe(this, [=]{ update_all(); }, song->MESSAGE_FINISHED_LOADING);

	set_mode(MixerMode::VOLUME);

	device_manager->subscribe(this, [=]{ on_update_device_manager(); });
	load_data();
}

MixingConsole::~MixingConsole()
{
	//song->unsubscribe(this);
	view->unsubscribe(this);
	device_manager->unsubscribe(this);
	select_module(nullptr);
	for (TrackMixer *m: mixer)
		delete m;
	delete peak_meter;
}

void MixingConsole::on_output_volume()
{
	device_manager->set_output_volume(get_float(""));
}

void MixingConsole::set_mode(MixerMode _mode)
{
	mode = _mode;
	for (auto *m: mixer)
		m->set_mode(mode);
	check("show-vol", mode == MixerMode::VOLUME);
	check("show-fx", mode == MixerMode::EFFECTS);

}

void MixingConsole::load_data()
{
	// how many TrackMixers still match?
	int n_ok = 0;
	foreachi (auto *m, mixer, i){
		if (!m->vtrack)
			break;
		if (i >= view->vtrack.num)
			break;
		if (m->vtrack != view->vtrack[i])
			break;
		n_ok ++;
	}

	// delete non-matching
	for (int i=n_ok; i<mixer.num; i++){
		delete mixer[i];
		remove_control("separator-" + i2s(i));
	}
	mixer.resize(n_ok);

	// add new
	foreachi(AudioViewTrack *t, view->vtrack, i){
		if (i >= n_ok){
			TrackMixer *m = new TrackMixer(t, this);
			mixer.add(m);
			embed(m, id_inner, i*2, 0);
			add_separator("!vertical", i*2 + 1, 0, "separator-" + i2s(i));
			m->set_mode(mode);
		}
	}

	hide_control("link-volumes", mixer.num <= 1);
}

void MixingConsole::on_update_device_manager()
{
	set_float("output-volume", device_manager->get_output_volume());
}

void MixingConsole::on_tracks_change()
{
	load_data();
}

void MixingConsole::select_module(Module *m)
{
	if (selected_module)
		selected_module->unsubscribe(this);

	if (config_panel)
		delete config_panel;
	config_panel = nullptr;

	selected_module = m;

	string config_grid_id = "config-panel-grid";

	Track *track = nullptr;

	if (selected_module){
		for (auto *mm: mixer)
			if (mm->track->fx.find((AudioEffect*)selected_module) >= 0)
				track = mm->track;

		AudioEffect *fx = (AudioEffect*)m;
		config_panel = new FxPanel(session, this, fx,
				[track,fx](bool enabled){ track->enable_effect(fx, enabled); },
				[track,fx]{ track->delete_effect(fx); },
				[track,fx](const string &old_param){ track->edit_effect(fx, old_param); });
		embed(config_panel, config_grid_id, 0, 0);

		m->subscribe(this, [=]{ select_module(nullptr); }, m->MESSAGE_DELETE);
	}

	reveal("config-revealer", m);


	// make sure only 1 item is selected in lists
	for (auto *m: mixer)
		if (m->track != track)
			m->set_int("fx", -1);
}

void MixingConsole::update_all()
{
	for (auto *m: mixer)
		m->update();
}

void MixingConsole::on_show()
{
	peak_meter->enable(true);
}

void MixingConsole::on_hide()
{
	peak_meter->enable(false);
}
