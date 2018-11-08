/*
 * MarkerDialog.cpp
 *
 *  Created on: May 13, 2015
 *      Author: ankele
 */

#include "MarkerDialog.h"
#include "../../Data/Track.h"
#include "../../Data/TrackMarker.h"

MarkerDialog::MarkerDialog(hui::Window* _parent, Track* _t, const Range &_range, const TrackMarker *_marker):
	hui::Window("marker_dialog", _parent)
{
	track = _t;
	range = _range;
	marker = _marker;

	if (marker){
		set_string("text", marker->text);
		range = marker->range;

		enable("ok", true);
	}else{
		enable("ok", false);
	}

	event("text", std::bind(&MarkerDialog::on_edit, this));
	event("cancel", std::bind(&MarkerDialog::on_close, this));
	event("hui:close", std::bind(&MarkerDialog::on_close, this));
	event("ok", std::bind(&MarkerDialog::on_ok, this));
}

MarkerDialog::~MarkerDialog()
{
}

void MarkerDialog::on_edit()
{
	enable("ok", get_string("text").num > 0);
}

void MarkerDialog::on_ok()
{
	if (marker){
		track->edit_marker(marker, marker->range, get_string("text"));
	}else{
		track->add_marker(range, get_string("text"));
	}
	destroy();
}

void MarkerDialog::on_close()
{
	destroy();
}
