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
#include "../helper/VolumeControl.h"
#include "../../data/base.h"
#include "../../data/Track.h"
#include "../../data/TrackLayer.h"
#include "../../data/Song.h"
#include "../../data/Sample.h"
#include "../../data/SampleRef.h"
#include "../../Session.h"
#include "../../lib/hui/language.h"

namespace tsunami {

SampleRefConsole::SampleRefConsole(Session *session, SideBar *_bar):
	SideBarConsole(_("Sample properties"), "sample-ref-console", session, _bar),
	in_cur_sample_changed(this, [this] { on_view_cur_sample_change(); })
{
	from_resource("sample-ref-dialog");
	layer = nullptr;
	sample = nullptr;
	editing = false;

	volume_control = new VolumeControl(this, "volume-slider", "volume", "volume-unit");
	volume_control->out_volume >> create_data_sink<float>([this] (float v) {
		editing = true;
		if (sample)
			layer->edit_sample_ref(sample, v, sample->muted);
		editing = false;
	});
	volume_control->set_range(0, 4);

	event("mute", [this] { on_mute(); });
	event("track", [this] { on_track(); });
}

void SampleRefConsole::on_enter() {
	set_sample(view->cur_sample());
	view->out_cur_sample_changed >> in_cur_sample_changed;
}

void SampleRefConsole::on_leave() {
	view->unsubscribe(this);
	set_sample(nullptr);

}


void SampleRefConsole::on_name() {
	//sample->origin->name = GetString("");
}

void SampleRefConsole::on_mute() {
	if (!sample)
		return;
	editing = true;
	layer->edit_sample_ref(sample, sample->volume, is_checked(""));

	volume_control->enable(!sample->muted);
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

void SampleRefConsole::set_sample(SampleRef *s) {
	if (sample)
		sample->unsubscribe(this);

	sample = s;
	layer = nullptr;

	if (sample) {
		layer = s->layer;
		sample->out_death >> create_sink([this] {
			// FIXME should also happen via view->on_cur_sample_change, but does not
			set_sample(nullptr);
		});
		sample->out_changed >> create_sink([this] {
			on_update();
		});
		sample->out_changed_by_action >> create_sink([this] {
			on_update();
		});
	}
	load_data();
}

void SampleRefConsole::load_data() {
	enable("name", false);

	hide_control("g-no-sample", sample);
	hide_control("g-sample", !sample);

	if (!sample)
		return;
	set_string("name", sample->origin->name);
	check("mute", sample->muted);
	volume_control->set(sample->volume);
	volume_control->enable(!sample->muted);
	reset("track");
	for (Track *t: weak(song->tracks))
		add_string("track", t->nice_name());
	//setInt("track", sample->track_no);
}

void SampleRefConsole::on_view_cur_sample_change() {
	set_sample(view->cur_sample());
}

void SampleRefConsole::on_update() {
	if (!editing)
		load_data();
}

}
