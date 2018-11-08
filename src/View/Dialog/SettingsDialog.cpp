/*
 * SettingsDialog.cpp
 *
 *  Created on: 26.03.2012
 *      Author: michi
 */

#include "SettingsDialog.h"
#include "../../Tsunami.h"
#include "../../Session.h"
#include "../../TsunamiWindow.h"
#include "../BottomBar/MiniBar.h"
#include "../Helper/CpuDisplay.h"
#include "../../Device/InputStreamAudio.h"
#include "../../Device/DeviceManager.h"
#include "../../Stuff/Log.h"
#include "../Helper/Slider.h"
#include "../AudioView.h"


struct ApiDescription
{
	string name;
	DeviceManager::ApiType type;
	int mode;
	bool available;
};
extern ApiDescription api_descriptions[];

SettingsDialog::SettingsDialog(AudioView *_view, hui::Window *_parent):
	hui::Window("settings_dialog", _parent)
{
	view = _view;
	event("language", std::bind(&SettingsDialog::on_language, this));
	event("color_scheme", std::bind(&SettingsDialog::on_color_scheme, this));
	event("ogg_bitrate", std::bind(&SettingsDialog::on_ogg_bitrate, this));
	event("default_artist", std::bind(&SettingsDialog::on_default_artist, this));
	event("scroll_speed", std::bind(&SettingsDialog::on_scroll_speed, this));
	event("cpu_meter", std::bind(&SettingsDialog::on_cpu_meter, this));
	event("audio_api", std::bind(&SettingsDialog::on_audio_api, this));
	event("midi_api", std::bind(&SettingsDialog::on_midi_api, this));
	event("hui:close", std::bind(&SettingsDialog::destroy, this));
	event("close", std::bind(&SettingsDialog::destroy, this));

	//setOptions("capture_filename", "placeholder=" + InputStreamAudio::getDefaultBackupFilename());
	set_options("default_artist", "placeholder=" + AppName);

	ogg_quality.add(OggQuality(0.0f, 64));
	ogg_quality.add(OggQuality(0.1f, 80));
	ogg_quality.add(OggQuality(0.2f, 96));
	ogg_quality.add(OggQuality(0.3f, 112));
	ogg_quality.add(OggQuality(0.4f, 128));
	ogg_quality.add(OggQuality(0.5f, 160));
	ogg_quality.add(OggQuality(0.6f, 192));
	ogg_quality.add(OggQuality(0.7f, 224));
	ogg_quality.add(OggQuality(0.8f, 256));
	ogg_quality.add(OggQuality(0.9f, 320));
	ogg_quality.add(OggQuality(1.0f, 500));

	loadData();

}

void SettingsDialog::loadData()
{
	// language
	Array<string> lang = hui::GetLanguages();
	foreachi(string &l, lang, i){
		add_string("language", l);
		if (l == hui::GetCurLanguage())
			set_int("language", i);
	}

	// color scheme
	foreachi(ColorSchemeBasic &b, view->basic_schemes, i){
		add_string("color_scheme", b.name);
		if (b.name == view->colors.name)
			set_int("color_scheme", i);
	}

	// ogg quality
	float CurOggQuality = hui::Config.get_float("OggQuality", 0.5f);
	foreachi(OggQuality &q, ogg_quality, i)
		if (CurOggQuality > q.quality - 0.05f)
			set_int("ogg_bitrate", i);
	set_decimals(1);

	set_string("default_artist", hui::Config.get_str("DefaultArtist", ""));

	//SetInt("preview_sleep", PreviewSleepTime);

	check("cpu_meter", hui::Config.get_bool("CpuDisplay", false));
	set_float("scroll_speed", hui::Config.get_float("View.MouseWheelSpeed", 1.0f));
	//enable("scroll_speed", false);

	int n_audio = 0, n_midi = 0;
	for (int i=0; i<(int)DeviceManager::ApiType::NUM_APIS; i++){
		auto &a = api_descriptions[i];
		if (!a.available)
			continue;
		if (a.mode & 1){
			add_string("audio_api", a.name);
			if (a.type == Session::GLOBAL->device_manager->audio_api)
				set_int("audio_api", n_audio);
			n_audio ++;
		}
		if (a.mode & 2){
			add_string("midi_api", a.name);
			if (a.type == Session::GLOBAL->device_manager->midi_api)
				set_int("midi_api", n_midi);
			n_midi ++;
		}
	}
}

void SettingsDialog::applyData()
{
}

void SettingsDialog::on_language()
{
	Array<string> lang = hui::GetLanguages();
	int l = get_int("");
	hui::SetLanguage(lang[l]);
	hui::Config.set_str("Language", lang[l]);
}

void SettingsDialog::on_color_scheme()
{
	int i = get_int("");
	if ((i >= 0) and (i < view->basic_schemes.num))
		view->set_color_scheme(view->basic_schemes[i].name);
}

void SettingsDialog::on_ogg_bitrate()
{
	hui::Config.set_float("OggQuality", ogg_quality[get_int("")].quality);
}

void SettingsDialog::on_default_artist()
{
	hui::Config.set_str("DefaultArtist", get_string(""));
}

void SettingsDialog::on_scroll_speed()
{
	view->mouse_wheel_speed = get_float("");
	hui::Config.set_float("View.MouseWheelSpeed", get_float(""));
}

void SettingsDialog::on_audio_api()
{
	int n = get_int("");
	int n_audio = 0;
	for (int i=0; i<(int)DeviceManager::ApiType::NUM_APIS; i++){
		auto &a = api_descriptions[i];
		if (!a.available)
			continue;
		if (a.mode & 1){
			if (n_audio == n)
				hui::Config.set_str("AudioApi", a.name);
			n_audio ++;
		}
	}
}

void SettingsDialog::on_midi_api()
{
	int n = get_int("");
	int n_midi = 0;
	for (int i=0; i<(int)DeviceManager::ApiType::NUM_APIS; i++){
		auto &a = api_descriptions[i];
		if (!a.available)
			continue;
		if (a.mode & 2){
			if (n_midi == n)
				hui::Config.set_str("MidiApi", a.name);
			n_midi ++;
		}
	}
}

void SettingsDialog::on_cpu_meter()
{
	bool show = is_checked("");
	hui::Config.set_bool("CpuDisplay", show);
	view->win->mini_bar->cpu_display->panel->hide_control(view->win->mini_bar->cpu_display->id, !show);
}
