/*
 * RawConfigDialog.cpp
 *
 *  Created on: 07.07.2013
 *      Author: michi
 */

#include "RawConfigDialog.h"
#include "../../Data/Track.h"
#include "../../Data/base.h"

RawConfigDialog::RawConfigDialog(RawConfigData *_data, hui::Window *parent) :
	hui::Window("raw_config_dialog", parent)
{
	data = _data;
	data->channels = 1;
	data->format = SampleFormat::SAMPLE_FORMAT_8;
	data->sample_rate = DEFAULT_SAMPLE_RATE;
	data->offset = 0;
	ok = false;

	for (int i=1;i<(int)SampleFormat::NUM_SAMPLE_FORMATS;i++)
		addString("format", format_name((SampleFormat)i));
	setInt("format", 0);

	event("hui:close", std::bind(&RawConfigDialog::on_close, this));
	event("close", std::bind(&RawConfigDialog::on_close, this));
	event("cancel", std::bind(&RawConfigDialog::on_close, this));
	event("ok", std::bind(&RawConfigDialog::on_ok, this));
}

RawConfigDialog::~RawConfigDialog()
{
}

void RawConfigDialog::on_close()
{
	destroy();
}

void RawConfigDialog::on_ok()
{
	data->format = (SampleFormat)(getInt("format") + 1);
	data->channels = getInt("channels") + 1;
	data->sample_rate = getInt("sample_rate");
	data->offset = getInt("offset");
	ok = true;
	destroy();
}
