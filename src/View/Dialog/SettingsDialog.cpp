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
	int index;
	int mode;
	bool available;
};
extern ApiDescription api_descriptions[];

SettingsDialog::SettingsDialog(AudioView *_view, hui::Window *_parent):
	hui::Window("settings_dialog", _parent)
{
	view = _view;
	event("language", std::bind(&SettingsDialog::onLanguage, this));
	event("color_scheme", std::bind(&SettingsDialog::onColorScheme, this));
	event("ogg_bitrate", std::bind(&SettingsDialog::onOggBitrate, this));
	event("default_artist", std::bind(&SettingsDialog::onDefaultArtist, this));
	event("scroll_speed", std::bind(&SettingsDialog::onScrollSpeed, this));
	event("cpu_meter", std::bind(&SettingsDialog::onCpuMeter, this));
	event("audio_api", std::bind(&SettingsDialog::onAudioApi, this));
	event("midi_api", std::bind(&SettingsDialog::onMidiApi, this));
	event("hui:close", std::bind(&SettingsDialog::destroy, this));
	event("close", std::bind(&SettingsDialog::destroy, this));

	//setOptions("capture_filename", "placeholder=" + InputStreamAudio::getDefaultBackupFilename());
	setOptions("default_artist", "placeholder=" + AppName);

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
		addString("language", l);
		if (l == hui::GetCurLanguage())
			setInt("language", i);
	}

	// color scheme
	foreachi(ColorSchemeBasic &b, view->basic_schemes, i){
		addString("color_scheme", b.name);
		if (b.name == view->colors.name)
			setInt("color_scheme", i);
	}

	// ogg quality
	float CurOggQuality = hui::Config.getFloat("OggQuality", 0.5f);
	foreachi(OggQuality &q, ogg_quality, i)
		if (CurOggQuality > q.quality - 0.05f)
			setInt("ogg_bitrate", i);
	setDecimals(1);

	setString("default_artist", hui::Config.getStr("DefaultArtist", ""));

	//SetInt("preview_sleep", PreviewSleepTime);

	check("cpu_meter", hui::Config.getBool("CpuDisplay", false));
	setFloat("scroll_speed", hui::Config.getFloat("View.MouseWheelSpeed", 1.0f));
	//enable("scroll_speed", false);

	int n_audio = 0, n_midi = 0;
	for (int i=0; i<DeviceManager::NUM_APIS; i++){
		auto &a = api_descriptions[i];
		if (!a.available)
			continue;
		if (a.mode & 1){
			addString("audio_api", a.name);
			if (a.index == Session::GLOBAL->device_manager->audio_api)
				setInt("audio_api", n_audio);
			n_audio ++;
		}
		if (a.mode & 2){
			addString("midi_api", a.name);
			if (a.index == Session::GLOBAL->device_manager->midi_api)
				setInt("midi_api", n_midi);
			n_midi ++;
		}
	}
}

void SettingsDialog::applyData()
{
}

void SettingsDialog::onLanguage()
{
	Array<string> lang = hui::GetLanguages();
	int l = getInt("");
	hui::SetLanguage(lang[l]);
	hui::Config.setStr("Language", lang[l]);
}

void SettingsDialog::onColorScheme()
{
	int i = getInt("");
	if ((i >= 0) and (i < view->basic_schemes.num))
		view->setColorScheme(view->basic_schemes[i].name);
}

void SettingsDialog::onOggBitrate()
{
	hui::Config.setFloat("OggQuality", ogg_quality[getInt("")].quality);
}

void SettingsDialog::onDefaultArtist()
{
	hui::Config.setStr("DefaultArtist", getString(""));
}

void SettingsDialog::onScrollSpeed()
{
	view->mouse_wheel_speed = getFloat("");
	hui::Config.setFloat("View.MouseWheelSpeed", getFloat(""));
}

void SettingsDialog::onAudioApi()
{
	int n = getInt("");
	int n_audio = 0;
	for (int i=0; i<DeviceManager::NUM_APIS; i++){
		auto &a = api_descriptions[i];
		if (!a.available)
			continue;
		if (a.mode & 1){
			if (n_audio == n)
				hui::Config.setStr("AudioApi", a.name);
			n_audio ++;
		}
	}
}

void SettingsDialog::onMidiApi()
{
	int n = getInt("");
	int n_midi = 0;
	for (int i=0; i<DeviceManager::NUM_APIS; i++){
		auto &a = api_descriptions[i];
		if (!a.available)
			continue;
		if (a.mode & 2){
			if (n_midi == n)
				hui::Config.setStr("MidiApi", a.name);
			n_midi ++;
		}
	}
}

void SettingsDialog::onCpuMeter()
{
	bool show = isChecked("");
	hui::Config.setBool("CpuDisplay", show);
	view->win->mini_bar->cpu_display->panel->hideControl(view->win->mini_bar->cpu_display->id, !show);
}
