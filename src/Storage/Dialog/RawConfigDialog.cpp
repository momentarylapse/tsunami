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

	addGrid("", 0, 0, 1, 2, "table1");
	setTarget("table1", 0);
	addGrid("", 0, 0, 2, 4, "table2");
	addGrid("", 0, 1, 2, 1, "table3");
	setTarget("table2", 0);
	addText(_("Format"), 0, 0, 0, 0, "");
	addComboBox("", 1, 0, 0, 0, "format");
	addText(_("Kan&ale"), 0, 1, 0, 0, "");
	addComboBox("Mono\\Stereo", 1, 1, 0, 0, "channels");
	addText(_("Samplerate"), 0, 2, 0, 0, "");
	addSpinButton("44100\\1", 1, 2, 0, 0, "sample_rate");
	addText(_("Offset"), 0, 3, 0, 0, "");
	addSpinButton("0\\0", 1, 3, 0, 0, "offset");
	setTarget("table3", 0);
	addButton(_("Abbrechen"), 0, 0, 0, 0, "cancel");
	addButton(_("Ok"), 1, 0, 0, 0, "ok");

	for (int i=1;i<NUM_SAMPLE_FORMATS;i++)
		addString("format", format_name((SampleFormat)i));
	setInt("format", 0);

	event("hui:close", this, &RawConfigDialog::onClose);
	event("close", this, &RawConfigDialog::onClose);
	event("ok", this, &RawConfigDialog::onOk);
}

RawConfigDialog::~RawConfigDialog()
{
}

void RawConfigDialog::onClose()
{
	delete(this);
}

void RawConfigDialog::onOk()
{
	data->format = (SampleFormat)(getInt("format") + 1);
	data->channels = getInt("channels") + 1;
	data->sample_rate = getInt("sample_rate");
	data->offset = getInt("offset");
	delete(this);
}
