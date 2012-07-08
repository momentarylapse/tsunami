/*
 * SettingsDialog.cpp
 *
 *  Created on: 26.03.2012
 *      Author: michi
 */

#include "SettingsDialog.h"
#include "../../Tsunami.h"

SettingsDialog::SettingsDialog(CHuiWindow *_parent, bool _allow_parent):
	CHuiWindow("dummy", -1, -1, 800, 600, _parent, _allow_parent, HuiWinModeControls, true)
{
	FromResource("properties_dialog");

	EventM("language", this, (void(HuiEventHandler::*)())&SettingsDialog::OnLanguage);
	EventM("ogg_bitrate", this, (void(HuiEventHandler::*)())&SettingsDialog::OnOggBitrate);
	EventM("preview_sleep", this, (void(HuiEventHandler::*)())&SettingsDialog::OnPreviewSleep);
	EventM("preview_device", this, (void(HuiEventHandler::*)())&SettingsDialog::OnPreviewDevice);
	EventM("capture_delay", this, (void(HuiEventHandler::*)())&SettingsDialog::OnCaptureDelay);
	EventM("hui:close", this, (void(HuiEventHandler::*)())&SettingsDialog::OnClose);
	EventM("close", this, (void(HuiEventHandler::*)())&SettingsDialog::OnClose);

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

	LoadData();

}

SettingsDialog::~SettingsDialog()
{
}

void SettingsDialog::LoadData()
{
	Array<string> lang = HuiGetLanguages();
	foreachi(lang, l, i){
		SetString("language", l);
		if (l == HuiGetCurLanguage())
			SetInt("language", i);
	}
	float CurOggQuality = HuiConfigReadFloat("OggQuality", 0.5f);
	foreachi(ogg_quality, q, i)
		if (CurOggQuality > q.quality - 0.05f)
			SetInt("ogg_bitrate", i);
	SetDecimals(1);
	//volume_slider = new Slider(this, "volume_slider", "volume", 0, 2, 100, (void(HuiEventHandler::*)())&TrackDialog::OnVolume, t->volume);
	//AddSlider(SettingsDialog, "volume_slider", "volume", 0, 2, 100, &OnSettingsVolume, Preview.volume);
	//tsunami->output->
	//SetInt("preview_sleep", PreviewSleepTime);
	SetString("preview_device", _("- Standard -"));
	SetInt("preview_device", 0);
	foreachi(tsunami->output->Device, d, i){
		AddString("preview_device", d);
		if (d == tsunami->output->ChosenDevice)
			SetInt("preview_device", i + 1);
	}
	SetFloat("capture_delay", tsunami->input->CapturePlaybackDelay);
}

void SettingsDialog::ApplyData()
{
}

void SettingsDialog::OnLanguage()
{
	Array<string> lang = HuiGetLanguages();
	int l = GetInt("");
	HuiSetLanguage(lang[l]);
	HuiConfigWriteStr("Language", lang[l]);
}

void SettingsDialog::OnOggBitrate()
{
	HuiConfigWriteFloat("OggQuality", ogg_quality[GetInt("")].quality);
}

void SettingsDialog::OnPreviewSleep()
{
	//PreviewSleepTime = GetInt("");
	//HuiConfigWriteInt("PreviewSleepTime", PreviewSleepTime);
}

void SettingsDialog::OnPreviewDevice()
{
	int dev = GetInt("");
	if (dev > 0)
		tsunami->output->ChosenDevice = tsunami->output->Device[dev - 1];
	else
		tsunami->output->ChosenDevice = "";
	HuiConfigWriteStr("ChosenOutputDevice", tsunami->output->ChosenDevice);
	HuiSaveConfigFile();
	tsunami->log->Warning(_("Das neue Ger&at wird erst beim n&achsten Start verwendet!"));
	//KillPreview();
	//PreviewInit();
}

void SettingsDialog::OnCaptureDelay()
{
	tsunami->input->CapturePlaybackDelay = GetFloat("");
	HuiConfigWriteFloat("CapturePlaybackDelay", tsunami->input->CapturePlaybackDelay);
}

void SettingsDialog::OnClose()
{
	delete(this);
}

