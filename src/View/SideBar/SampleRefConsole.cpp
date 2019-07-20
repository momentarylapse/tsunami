/*
 * SampleRefConsole.cpp
 *
 *  Created on: 26.03.2012
 *      Author: michi
 */

#include "../AudioView.h"
#include "../../Data/base.h"
#include "../../Data/Track.h"
#include "../../Data/TrackLayer.h"
#include "../../Data/Song.h"
#include "../../Data/Sample.h"
#include "../../Data/SampleRef.h"
#include "../../Session.h"
#include "../Helper/Slider.h"
#include "SampleRefConsole.h"
#include "SampleManagerConsole.h"

SampleRefConsole::SampleRefConsole(Session *session):
	SideBarConsole(_("Sample properties"), session)
{
	from_resource("sample_ref_dialog");
	layer = nullptr;
	sample = nullptr;

	event("volume", [=]{ on_volume(); });
	event("mute", [=]{ on_mute(); });
	event("track", [=]{ on_track(); });

	event("edit_song", [=]{ on_edit_song(); });
	event("edit_track", [=]{ on_edit_track(); });
	event("edit_sample", [=]{ on_edit_sample(); });

	view->subscribe(this, [=]{ on_view_cur_sample_change(); }, view->MESSAGE_CUR_SAMPLE_CHANGE);
}

SampleRefConsole::~SampleRefConsole()
{
	if (sample)
		sample->unsubscribe(this);
	view->unsubscribe(this);
}


void SampleRefConsole::on_name()
{
	//sample->origin->name = GetString("");
}

void SampleRefConsole::on_mute()
{
	if (!sample)
		return;
	sample->unsubscribe(this);
	layer->edit_sample_ref(sample, sample->volume, is_checked(""));

	enable("volume", !sample->muted);
	sample->subscribe(this, [=]{ on_update(); });
}

void SampleRefConsole::on_track()
{
	//int n = getInt("");
}

void SampleRefConsole::on_volume()
{
	if (!sample)
		return;
	sample->unsubscribe(this);
	layer->edit_sample_ref(sample, db2amplitude(get_float("")), sample->muted);
	sample->subscribe(this, [=]{ on_update(); });
}

void SampleRefConsole::on_edit_song()
{
	session->set_mode("default/song");
}

void SampleRefConsole::on_edit_track()
{
	session->set_mode("default/track");
}

void SampleRefConsole::on_edit_sample()
{
	bar()->sample_manager->set_selection({sample->origin});
	session->set_mode("default/samples");
}

void SampleRefConsole::load_data()
{
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
	for (Track *t: song->tracks)
		add_string("track", t->nice_name());
	//setInt("track", sample->track_no);
}

void SampleRefConsole::on_view_cur_sample_change()
{
	if (sample)
		sample->unsubscribe(this);
	layer = view->cur_layer();
	sample = view->cur_sample();
	if (sample)
		sample->subscribe(this, [=]{ on_update(); });
	load_data();
}

void SampleRefConsole::on_update()
{
	if (sample->cur_message() == sample->MESSAGE_DELETE){
		sample->unsubscribe(this);
		sample = nullptr;
	}
	load_data();
}
