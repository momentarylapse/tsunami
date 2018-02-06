/*
 * SettingsDialog.cpp
 *
 *  Created on: 26.03.2012
 *      Author: michi
 */

#include "SettingsDialog.h"
#include "../../Tsunami.h"
#include "../../Device/InputStreamAudio.h"
#include "../../Device/DeviceManager.h"
#include "../../Stuff/Log.h"
#include "../Helper/Slider.h"
#include "../AudioView.h"

SettingsDialog::SettingsDialog(AudioView *_view, hui::Window *_parent):
	hui::ResourceWindow("settings_dialog", _parent)
{
	view = _view;
	event("language", std::bind(&SettingsDialog::onLanguage, this));
	event("color_scheme", std::bind(&SettingsDialog::onColorScheme, this));
	event("ogg_bitrate", std::bind(&SettingsDialog::onOggBitrate, this));
	event("default_artist", std::bind(&SettingsDialog::onDefaultArtist, this));
	event("capture_filename", std::bind(&SettingsDialog::onCaptureFilename, this));
	event("capture_find", std::bind(&SettingsDialog::onCaptureFind, this));
	event("hui:close", std::bind(&SettingsDialog::destroy, this));
	event("close", std::bind(&SettingsDialog::destroy, this));

	setOptions("capture_filename", "placeholder=" + InputStreamAudio::getDefaultBackupFilename());
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

	setString("capture_filename", InputStreamAudio::backup_filename);
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

void SettingsDialog::onCaptureFilename()
{
	InputStreamAudio::setBackupFilename(getString(""));
}

void SettingsDialog::onCaptureFind()
{
	if (hui::FileDialogSave(this, _("Select backup file for recordings"), InputStreamAudio::backup_filename.basename(), "*.raw", "*.raw"))
		setString("capture_filename", hui::Filename);
	InputStreamAudio::setBackupFilename(hui::Filename);
}

