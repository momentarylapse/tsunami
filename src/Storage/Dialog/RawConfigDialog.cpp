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
		add_string("format", format_name((SampleFormat)i));
	set_int("format", 0);

	event("hui:close", [=]{ on_close(); });
	event("close", [=]{ on_close(); });
	event("cancel", [=]{ on_close(); });
	event("ok", [=]{ on_ok(); });
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
	data->format = (SampleFormat)(get_int("format") + 1);
	data->channels = get_int("channels") + 1;
	data->sample_rate = get_int("sample_rate");
	data->offset = get_int("offset");
	ok = true;
	destroy();
}
