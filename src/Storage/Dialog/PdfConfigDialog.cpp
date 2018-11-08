/*
 * PdfConfigDialog.cpp
 *
 *  Created on: 25.08.2018
 *      Author: michi
 */

#include "PdfConfigDialog.h"
#include "../../Data/base.h"
#include "../../Data/Song.h"
#include "../../Data/Track.h"

PdfConfigDialog::PdfConfigDialog(PdfConfigData *_data, Song *_song, hui::Window *parent) :
	hui::Dialog("pdf export config", 200, 100, parent, false)
{
	data = _data;
	song = _song;
	ok = false;

	addGrid("", 0, 0, "root");
	setTarget("root");
	addGrid("", 0, 0, "stuff");
	addGrid("", 0, 1, "tracks");
	addGrid("!button-bar", 0, 2, "buttons");
	setTarget("buttons");
	addButton(_("Cancel"), 0, 0, "cancel");
	addButton(_("Ok"), 1, 0, "ok");
	setTarget("stuff");
	addLabel("scale", 0, 0, "");
	addSpinButton("!expandx", 1, 0, "scale");
	addLabel("%", 2, 0, "");
	setTarget("tracks");
	foreachi(Track *t, song->tracks, i){
		if (t->type != SignalType::MIDI)
			continue;
		addLabel(t->nice_name(), 0, i, "");
		addCheckBox("Classical", 1, i, format("classical-%d"));
		addCheckBox("TAB", 2, i, format("tab-%d"));
		check(format("classical-%d"), true);
		check(format("tab-%d"), t->instrument.string_pitch.allocated > 0);
		enable(format("tab-%d"), t->instrument.string_pitch.allocated > 0);
	}
	setFloat("scale", 100.0f);

	event("hui:close", [&]{ on_close(); });
	event("cancel", [&]{ on_close(); });
	event("ok", [&]{ on_ok(); });
}

PdfConfigDialog::~PdfConfigDialog()
{
}

void PdfConfigDialog::on_close()
{
	destroy();
}

void PdfConfigDialog::on_ok()
{
	ok = true;
	data->track_mode.resize(song->tracks.num);
	foreachi(Track *t, song->tracks, i){
		if (t->type != SignalType::MIDI)
			continue;
		bool classical = isChecked(format("classical-%d"));
		bool tab = isChecked(format("tab-%d"));
		data->track_mode[i] = (classical ? 1 : 0) + (tab ? 2 : 0);
	}
	data->horizontal_scale = getFloat("scale") / 100;

	destroy();
}

