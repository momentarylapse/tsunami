/*
 * SettingsDialog.cpp
 *
 *  Created on: 26.03.2012
 *      Author: michi
 */

#include "SettingsDialog.h"
#include "../../Tsunami.h"
#include "../../Audio/AudioOutput.h"
#include "../../Audio/AudioInput.h"
#include "../../Audio/AudioInputAudio.h"
#include "../../Stuff/Log.h"
#include "../Helper/Slider.h"

SettingsDialog::SettingsDialog(HuiWindow *_parent, bool _allow_parent):
	HuiWindow("settings_dialog", _parent, _allow_parent)
{
	EventM("language", this, &SettingsDialog::onLanguage);
	EventM("ogg_bitrate", this, &SettingsDialog::onOggBitrate);
	EventM("preview_device", this, &SettingsDialog::onPreviewDevice);
	EventM("capture_device", this, &SettingsDialog::onCaptureDevice);
	EventM("capture_delay", this, &SettingsDialog::onCaptureDelay);
	EventM("capture_filename", this, &SettingsDialog::onCaptureFilename);
	EventM("capture_find", this, &SettingsDialog::onCaptureFind);
	EventM("hui:close", this, &SettingsDialog::onClose);
	EventM("close", this, &SettingsDialog::onClose);

	SetOptions("capture_filename", "placeholder=" + tsunami->input->in_audio->getDefaultTempFilename());

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
	Array<string> lang = HuiGetLanguages();
	foreachi(string &l, lang, i){
		SetString("language", l);
		if (l == HuiGetCurLanguage())
			SetInt("language", i);
	}
	float CurOggQuality = HuiConfig.getFloat("OggQuality", 0.5f);
	foreachi(OggQuality &q, ogg_quality, i)
		if (CurOggQuality > q.quality - 0.05f)
			SetInt("ogg_bitrate", i);
	SetDecimals(1);
	//volume_slider = new Slider(this, "volume_slider", "volume", 0, 2, 100, &TrackDialog::onVolume, t->volume);
	//AddSlider(SettingsDialog, "volume_slider", "volume", 0, 2, 100, &OnSettingsVolume, Preview.volume);
	//tsunami->output->
	//SetInt("preview_sleep", PreviewSleepTime);
	SetString("preview_device", _("- Standard -"));
	SetInt("preview_device", 0);
	foreachi(string &d, tsunami->output->Device, i){
		AddString("preview_device", d);
		if (d == tsunami->output->ChosenDevice)
			SetInt("preview_device", i + 1);
	}


	SetString("capture_device", _("- Standard -"));
	SetInt("capture_device", 0);
	foreachi(string &d, tsunami->input->in_audio->Device, i){
		AddString("capture_device", d);
		if (d == tsunami->input->in_audio->ChosenDevice)
			SetInt("capture_device", i + 1);
	}

	SetFloat("capture_delay", tsunami->input->in_audio->getPlaybackDelayConst());

	SetString("capture_filename", tsunami->input->in_audio->TempFilename);
}

void SettingsDialog::applyData()
{
}

void SettingsDialog::onLanguage()
{
	Array<string> lang = HuiGetLanguages();
	int l = GetInt("");
	HuiSetLanguage(lang[l]);
	HuiConfig.setStr("Language", lang[l]);
}

void SettingsDialog::onOggBitrate()
{
	HuiConfig.setFloat("OggQuality", ogg_quality[GetInt("")].quality);
}

void SettingsDialog::onCaptureDevice()
{
	if (GetInt("") > 0)
		tsunami->input->in_audio->ChosenDevice = tsunami->input->in_audio->Device[GetInt("") - 1];
	else
		tsunami->input->in_audio->ChosenDevice = "";
	HuiConfig.setStr("Input.ChosenDevice", tsunami->input->in_audio->ChosenDevice);
}

void SettingsDialog::onPreviewDevice()
{
	int dev = GetInt("");
	if (dev > 0)
		tsunami->output->setDevice(tsunami->output->Device[dev - 1]);
	else
		tsunami->output->setDevice("");
}

void SettingsDialog::onCaptureDelay()
{
	tsunami->input->in_audio->setPlaybackDelayConst(GetFloat(""));
}

void SettingsDialog::onCaptureFilename()
{
	tsunami->input->in_audio->setTempFilename(GetString(""));
}

void SettingsDialog::onCaptureFind()
{
	if (HuiFileDialogSave(this, _("Sicherungsdatei f&ur Aufnahmen w&ahlen"), tsunami->input->in_audio->TempFilename.basename(), "*.raw", "*.raw"))
		SetString("capture_filename", HuiFilename);
		tsunami->input->in_audio->setTempFilename(HuiFilename);
}

void SettingsDialog::onClose()
{
	delete(this);
}

