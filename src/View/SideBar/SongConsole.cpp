/*
 * SongConsole.cpp
 *
 *  Created on: 26.03.2012
 *      Author: michi
 */

#include "../../Session.h"
#include "../../EditModes.h"
#include "../../View/AudioView.h"
#include "../BottomBar/BottomBar.h"
#include "../../Data/Song.h"
#include "../../Data/base.h"
#include "SongConsole.h"

const int NUM_POSSIBLE_FORMATS = 4;
const SampleFormat POSSIBLE_FORMATS[NUM_POSSIBLE_FORMATS] = {
	SampleFormat::SAMPLE_FORMAT_16,
	SampleFormat::SAMPLE_FORMAT_24,
	SampleFormat::SAMPLE_FORMAT_32,
	SampleFormat::SAMPLE_FORMAT_32_FLOAT
};

SongConsole::SongConsole(Session *session, SideBar *bar) :
	SideBarConsole(_("File"), "song-console", session, bar)
{
	// dialog
	embed_dialog("song_dialog", 0, 0);
	set_decimals(1);

	expand("ad_t_tags", 0, true);

	add_string("samplerate", "22050");
	add_string("samplerate", i2s(DEFAULT_SAMPLE_RATE));
	add_string("samplerate", "48000");
	add_string("samplerate", "96000");

	for (int i=0; i<NUM_POSSIBLE_FORMATS; i++)
		add_string("format", format_name(POSSIBLE_FORMATS[i]));

	load_data();

	menu_tags = hui::create_resource_menu("popup-menu-tag", this);

	event("samplerate", [this]{ on_samplerate(); });
	event("format", [this]{ on_format(); });
	event_x("tags", "hui:change", [this]{ on_tags_edit(); });
	event_x("tags", "hui:right-button-down", [this]{ on_tags_right_click(); });
	event("tag-add", [this]{ on_tag_add(); });
	event("tag-delete", [this]{ on_tag_delete(); });

	event("edit_track", [session] {
		session->set_mode(EditMode::DefaultTrack);
	});
	event("edit_samples", [session] {
		session->set_mode(EditMode::DefaultSamples);
	});

	song->subscribe(this, [this]{ on_update(); }, song->MESSAGE_ANY);
}

SongConsole::~SongConsole() {
	song->unsubscribe(this);
	delete menu_tags;
}

void SongConsole::load_data() {
	// tags
	reset("tags");
	for (Tag &t: song->tags)
		add_string("tags", t.key + "\\" + t.value);
	enable("delete_tag", false);

	// data
	int samples = song->range().length;
	set_string("start", song->get_time_str_long(song->range().start()));
	set_string("end", song->get_time_str_long(song->range().end()));
	set_string("length", song->get_time_str_long(samples));
	set_string("samples", i2s(samples));

	set_string("samplerate", i2s(song->sample_rate));
	for (int i=0; i<NUM_POSSIBLE_FORMATS; i++)
		if (song->default_format == POSSIBLE_FORMATS[i])
			set_int("format", i);
}

void SongConsole::on_samplerate() {
	song->set_sample_rate(get_string("")._int());
}

void SongConsole::on_format() {
	int i = get_int("");
	if (i >= 0)
		song->set_default_format(POSSIBLE_FORMATS[i]);
}

void SongConsole::on_tags_edit() {
	int r = hui::get_event()->row;
	if (r < 0)
		return;
	Tag t = song->tags[r];
	if (hui::get_event()->column == 0)
		t.key = get_cell("tags", r, 0);
	else
		t.value = get_cell("tags", r, 1);
	song->edit_tag(r, t.key, t.value);
}

void SongConsole::on_tags_right_click() {
	int n = hui::get_event()->row;
	menu_tags->enable("tag-delete", n >= 0);
	menu_tags->open_popup(this);
}

void SongConsole::on_tag_add() {
	song->add_tag("key", "value");
}

void SongConsole::on_tag_delete() {
	int s = get_int("tags");
	if (s >= 0)
		song->delete_tag(s);
}

void SongConsole::on_update() {
	load_data();
}
