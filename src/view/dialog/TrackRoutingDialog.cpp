/*
 * TrackRoutingDialog.cpp
 *
 *  Created on: 11.06.2019
 *      Author: michi
 */

#include "TrackRoutingDialog.h"
#include "../../data/base.h"
#include "../../data/Song.h"
#include "../../data/Track.h"
#include "../../lib/hui/language.h"

namespace tsunami {

TrackRoutingDialog::TrackRoutingDialog(hui::Window *parent, Song *_song):
	hui::Dialog("track-routing-dialog", parent)
{
	song = _song;
	num_tracks = 0;

	load();
	event("close", [this] { request_destroy(); });
	event("add-group", [this] { on_add_group(); });
}

void TrackRoutingDialog::load() {
	groups.clear();

	for (Track *t: weak(song->tracks))
		if (t->type == SignalType::Group)
			groups.add(t);

	foreachi (Track *t, weak(song->tracks), i) {
		set_target("list");
		string id = "target-" + i2s(i);
		if (i >= num_tracks) {
			if (t->type == SignalType::Group)
				add_label("<i>" + t->nice_name() + "</i>", 0, i+1, "");
			else
				add_label(t->nice_name(), 0, i+1, "");
			add_combo_box("!expandx", 1, i+1, id);
			event(id, [this, i] { on_target(i); });
			num_tracks ++;
		}
		reset(id);
		add_string(id, _("- direct out -"));
		set_int(id, 0);
		foreachi (Track *g, groups, j) {
			add_string(id, g->nice_name());
			if (t->send_target == g)
				set_int(id, j + 1);
		}
	}
}

void TrackRoutingDialog::on_target(int i) {
	int j = get_int("");
	if (j >= 1)
		song->tracks[i]->set_send_target(groups[j - 1]);
	else
		song->tracks[i]->set_send_target(nullptr);
}

bool has_track_name(Song *s, const string &name) {
	for (auto *t: weak(s->tracks))
		if (t->name == name)
			return true;
	return false;
}

void TrackRoutingDialog::on_add_group() {
	song->begin_action_group("add track group");
	[[maybe_unused]] auto *t = song->add_track(SignalType::Group);
	song->end_action_group();
	load();
}

}
