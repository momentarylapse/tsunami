/*
 * RawConfigDialog.cpp
 *
 *  Created on: 07.07.2013
 *      Author: michi
 */

#include "RawConfigDialog.h"
#include "../../Data/Track.h"

RawConfigDialog::RawConfigDialog(RawConfigData *_data, HuiWindow *parent) :
	HuiDialog(_("Raw Daten"), 300, 100, parent, false)
{
	data = _data;
	data->channels = 1;
	data->format = SAMPLE_FORMAT_8;
	data->sample_rate = DEFAULT_SAMPLE_RATE;
	data->offset = 0;

	AddControlTable("", 0, 0, 1, 2, "table1");
	SetTarget("table1", 0);
	AddControlTable("", 0, 0, 2, 4, "table2");
	AddControlTable("", 0, 1, 2, 1, "table3");
	SetTarget("table2", 0);
	AddText(_("Format"), 0, 0, 0, 0, "");
	AddComboBox("", 1, 0, 0, 0, "format");
	AddText(_("Kan&ale"), 0, 1, 0, 0, "");
	AddComboBox("Mono\\Stereo", 1, 1, 0, 0, "channels");
	AddText(_("Samplerate"), 0, 2, 0, 0, "");
	AddSpinButton("44100\\1", 1, 2, 0, 0, "sample_rate");
	AddText(_("Offset"), 0, 3, 0, 0, "");
	AddSpinButton("0\\0", 1, 3, 0, 0, "offset");
	SetTarget("table3", 0);
	AddButton(_("Abbrechen"), 0, 0, 0, 0, "cancel");
	AddButton(_("Ok"), 1, 0, 0, 0, "ok");

	for (int i=1;i<NUM_SAMPLE_FORMATS;i++)
		AddString("format", format_name((SampleFormat)i));
	SetInt("format", 0);

	EventM("hui:close", this, &RawConfigDialog::OnClose);
	EventM("close", this, &RawConfigDialog::OnClose);
	EventM("ok", this, &RawConfigDialog::OnOk);
}

RawConfigDialog::~RawConfigDialog()
{
}

void RawConfigDialog::OnClose()
{
	delete(this);
}

void RawConfigDialog::OnOk()
{
	data->format = (SampleFormat)(GetInt("format") + 1);
	data->channels = GetInt("channels") + 1;
	data->sample_rate = GetInt("sample_rate");
	data->offset = GetInt("offset");
	delete(this);
}
