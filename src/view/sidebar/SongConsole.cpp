/*
 * SongConsole.cpp
 *
 *  Created on: 26.03.2012
 *      Author: michi
 */

#include "SongConsole.h"
#include "../audioview/AudioView.h"
#include "../bottombar/BottomBar.h"
#include "../../data/Song.h"
#include "../../data/base.h"
#include "../../lib/base/iter.h"
#include "../../Session.h"
#include "../../EditModes.h"

const Array<SampleFormat> POSSIBLE_FORMATS = {
	SampleFormat::INT_16,
	SampleFormat::INT_24,
	SampleFormat::INT_32,
	SampleFormat::FLOAT_32
};

SongConsole::SongConsole(Session *session, SideBar *bar) :
	SideBarConsole(_("File"), "song-console", session, bar)
{
	// dialog
	embed_dialog("song_dialog", 0, 0);
	set_decimals(1);

	expand_row("ad_t_tags", 0, true);

	for (auto rate: POSSIBLE_SAMPLE_RATES)
		add_string("samplerate", i2s(rate));

	for (auto f: POSSIBLE_FORMATS)
		add_string("format", format_name(f));

	menu_tags = hui::create_resource_menu("popup-menu-tag", this);

	event("samplerate", [this] { on_samplerate(); });
	event("format", [this] { on_format(); });
	event_x("tags", "hui:change", [this] { on_tags_edit(); });
	event_x("tags", "hui:right-button-down", [this] { on_tags_right_click(); });
	event("tag-add", [this] { on_tag_add(); });
	event("tag-delete", [this] { on_tag_delete(); });

	event("edit-track", [session] {
		session->set_mode(EditMode::DefaultTrack);
	});
	event("edit-samples", [session] {
		session->set_mode(EditMode::DefaultSamples);
	});
}

void SongConsole::on_enter() {
	song->subscribe(this, [this] { on_update(); }, song->MESSAGE_ANY);
	load_data();
}

void SongConsole::on_leave() {
	song->unsubscribe(this);
}

void SongConsole::load_data() {
	// tags
	reset("tags");
	for (Tag &t: song->tags)
		add_string("tags", t.key + "\\" + t.value);
	enable("delete_tag", false);

	// data
	auto r = song->range();
	int samples = r.length;
	set_string("start", song->get_time_str_long(r.start()));
	set_string("end", song->get_time_str_long(r.end()));
	set_string("length", song->get_time_str_long(samples));
	set_string("samples", i2s(samples));

	set_string("samplerate", i2s(song->sample_rate));
	for (const auto& [i, rate]: enumerate(POSSIBLE_SAMPLE_RATES))
		if (song->sample_rate == rate)
			set_int("samplerate", i);
	for (const auto& [i, f]: enumerate(POSSIBLE_FORMATS))
		if (song->default_format == f)
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
