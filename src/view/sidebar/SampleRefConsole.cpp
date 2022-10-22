/*
 * SampleRefConsole.cpp
 *
 *  Created on: 26.03.2012
 *      Author: michi
 */

#include "SampleRefConsole.h"
#include "SampleManagerConsole.h"
#include "../audioview/AudioView.h"
#include "../helper/Slider.h"
#include "../../data/base.h"
#include "../../data/Track.h"
#include "../../data/TrackLayer.h"
#include "../../data/Song.h"
#include "../../data/Sample.h"
#include "../../data/SampleRef.h"
#include "../../Session.h"
#include "../../EditModes.h"

SampleRefConsole::SampleRefConsole(Session *session, SideBar *_bar):
	SideBarConsole(_("Sample properties"), "sample-ref-console", session, _bar)
{
	from_resource("sample_ref_dialog");
	layer = nullptr;
	sample = nullptr;
	editing = false;

	event("volume", [this] { on_volume(); });
	event("mute", [this] { on_mute(); });
	event("track", [this] { on_track(); });

	event("edit_song", [session] {
		session->set_mode(EditMode::DefaultSong);
	});
	event("edit_track", [session] {
		session->set_mode(EditMode::DefaultTrack);
	});
	event("edit_samples", [this, session] {
		bar()->sample_manager->set_selection({sample->origin.get()});
		session->set_mode(EditMode::DefaultSamples);
	});
}

void SampleRefConsole::on_enter() {
	view->subscribe(this, [this] { on_view_cur_sample_change(); }, view->MESSAGE_CUR_SAMPLE_CHANGE);
}

void SampleRefConsole::on_leave() {
	if (sample)
		sample->unsubscribe(this);
	view->unsubscribe(this);
}


void SampleRefConsole::on_name() {
	//sample->origin->name = GetString("");
}

void SampleRefConsole::on_mute() {
	if (!sample)
		return;
	editing = true;
	layer->edit_sample_ref(sample, sample->volume, is_checked(""));

	enable("volume", !sample->muted);
	editing = true;
}

void SampleRefConsole::on_track() {
	//int n = getInt("");
}

void SampleRefConsole::on_volume() {
	if (!sample)
		return;
	editing = true;
	layer->edit_sample_ref(sample, db2amplitude(get_float("")), sample->muted);
	editing = false;
}

void SampleRefConsole::load_data() {
	enable("name", false);
	enable("mute", sample);
	enable("volume", sample);
	enable("repnum", sample);
	enable("repdelay", sample);

	set_string("name", _("no sample selected"));

	if (!sample)
		return;
	set_string("name", sample->origin->name);
	set_decimals(1);
	check("mute", sample->muted);
	set_float("volume", amplitude2db(sample->volume));
	enable("volume", !sample->muted);
	reset("track");
	for (Track *t: weak(song->tracks))
		add_string("track", t->nice_name());
	//setInt("track", sample->track_no);
}

void SampleRefConsole::on_view_cur_sample_change() {
	if (sample)
		sample->unsubscribe(this);
	layer = view->cur_layer();
	sample = view->cur_sample();
	if (sample) {
		sample->subscribe(this, [this] {
			sample->unsubscribe(this);
			sample = nullptr;
		}, sample->MESSAGE_DELETE);
		sample->subscribe(this, [this] { on_update(); }, sample->MESSAGE_ANY);
	}
	load_data();
}

void SampleRefConsole::on_update() {
	if (!editing)
		load_data();
}
