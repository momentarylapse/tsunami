/*
 * SettingsDialog.cpp
 *
 *  Created on: 26.03.2012
 *      Author: michi
 */

#include "SettingsDialog.h"
#include "../../Tsunami.h"
#include "../../Audio/AudioOutput.h"
#include "../../Audio/AudioInputAudio.h"
#include "../../Stuff/Log.h"
#include "../Helper/Slider.h"
#include "../AudioView.h"

SettingsDialog::SettingsDialog(HuiWindow *_parent, bool _allow_parent):
	HuiWindow("settings_dialog", _parent, _allow_parent)
{
	event("language", this, &SettingsDialog::onLanguage);
	event("color_scheme", this, &SettingsDialog::onColorScheme);
	event("ogg_bitrate", this, &SettingsDialog::onOggBitrate);
	event("default_artist", this, &SettingsDialog::onDefaultArtist);
	event("preview_device", this, &SettingsDialog::onPreviewDevice);
	event("capture_device", this, &SettingsDialog::onCaptureDevice);
	event("capture_delay", this, &SettingsDialog::onCaptureDelay);
	event("capture_filename", this, &SettingsDialog::onCaptureFilename);
	event("capture_find", this, &SettingsDialog::onCaptureFind);
	event("hui:close", this, &SettingsDialog::onClose);
	event("close", this, &SettingsDialog::onClose);

	setOptions("capture_filename", "placeholder=" + AudioInputAudio::getDefaultTempFilename());
	setOptions("default_artist", "placeholder=" + AppName);

	setTooltip("default_artist", _("wird als Tag artist=... in neue Dateien eingetragen"));
	setTooltip("capture_filename", _("Aufnahmen werden automatisch im Format Raw (stereo, 32bit float) in diese Datei gespeichert (und sp&ater gel&oscht), falls das Program w&ahrendher abst&urzt"));

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

SettingsDialog::~SettingsDialog()
{
}

void SettingsDialog::loadData()
{
	// language
	Array<string> lang = HuiGetLanguages();
	foreachi(string &l, lang, i){
		addString("language", l);
		if (l == HuiGetCurLanguage())
			setInt("language", i);
	}

	// color scheme
	foreachi(ColorSchemeBasic &b, tsunami->_view->basic_schemes, i){
		addString("color_scheme", b.name);
		if (b.name == tsunami->_view->colors.name)
			setInt("color_scheme", i);
	}

	// ogg quality
	float CurOggQuality = HuiConfig.getFloat("OggQuality", 0.5f);
	foreachi(OggQuality &q, ogg_quality, i)
		if (CurOggQuality > q.quality - 0.05f)
			setInt("ogg_bitrate", i);
	setDecimals(1);

	setString("default_artist", HuiConfig.getStr("DefaultArtist", ""));

	//SetInt("preview_sleep", PreviewSleepTime);

	setString("preview_device", _("- Standard -"));
	setInt("preview_device", 0);
	output_devices = tsunami->output->getDevices();
	output_devices.insert("", 0);
	foreachi(string &d, output_devices, i){
		if (i == 0)
			continue;
		addString("preview_device", d);
		if (d == tsunami->output->chosen_device)
			setInt("preview_device", i);
	}

	addString("capture_device", _("- Standard -"));
	setInt("capture_device", 0);
	capture_devices = AudioInputAudio::getDevices();
	capture_devices.insert("", 0);
	foreachi(string &d, capture_devices, i){
		if (i == 0)
			continue;
		addString("capture_device", d);
		if (d == AudioInputAudio::getFavoriteDevice())
			setInt("capture_device", i);
	}

	setFloat("capture_delay", AudioInputAudio::getPlaybackDelayConst());

	setString("capture_filename", AudioInputAudio::temp_filename);
}

void SettingsDialog::applyData()
{
}

void SettingsDialog::onLanguage()
{
	Array<string> lang = HuiGetLanguages();
	int l = getInt("");
	HuiSetLanguage(lang[l]);
	HuiConfig.setStr("Language", lang[l]);
}

void SettingsDialog::onColorScheme()
{
	int i = getInt("");
	if ((i >= 0) and (i < tsunami->_view->basic_schemes.num))
		tsunami->_view->setColorScheme(tsunami->_view->basic_schemes[i].name);
}

void SettingsDialog::onOggBitrate()
{
	HuiConfig.setFloat("OggQuality", ogg_quality[getInt("")].quality);
}

void SettingsDialog::onDefaultArtist()
{
	HuiConfig.setStr("DefaultArtist", getString(""));
}

void SettingsDialog::onCaptureDevice()
{
	int dev = getInt("");
	if (dev >= 0)
		AudioInputAudio::setFavoriteDevice(capture_devices[dev]);
}

void SettingsDialog::onPreviewDevice()
{
	int dev = getInt("");
	if (dev >= 0)
		tsunami->output->setDevice(output_devices[dev]);
}

void SettingsDialog::onCaptureDelay()
{
	AudioInputAudio::setPlaybackDelayConst(getFloat(""));
}

void SettingsDialog::onCaptureFilename()
{
	AudioInputAudio::setTempFilename(getString(""));
}

void SettingsDialog::onCaptureFind()
{
	if (HuiFileDialogSave(this, _("Sicherungsdatei f&ur Aufnahmen w&ahlen"), AudioInputAudio::temp_filename.basename(), "*.raw", "*.raw"))
		setString("capture_filename", HuiFilename);
	AudioInputAudio::setTempFilename(HuiFilename);
}

void SettingsDialog::onClose()
{
	delete(this);
}

