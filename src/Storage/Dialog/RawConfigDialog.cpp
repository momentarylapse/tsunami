/*
 * RawConfigDialog.cpp
 *
 *  Created on: 07.07.2013
 *      Author: michi
 */

#include "RawConfigDialog.h"
#include "../../Data/Track.h"

RawConfigDialog::RawConfigDialog(RawConfigData *_data, HuiWindow *parent) :
	HuiWindow("raw_config_dialog", parent)
{
	data = _data;
	data->channels = 1;
	data->format = SAMPLE_FORMAT_8;
	data->sample_rate = DEFAULT_SAMPLE_RATE;
	data->offset = 0;

	for (int i=1;i<NUM_SAMPLE_FORMATS;i++)
		addString("format", format_name((SampleFormat)i));
	setInt("format", 0);

	event("hui:close", this, &RawConfigDialog::onClose);
	event("close", this, &RawConfigDialog::onClose);
	event("cancel", this, &RawConfigDialog::onClose);
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
