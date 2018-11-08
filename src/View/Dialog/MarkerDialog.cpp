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
		setString("text", marker->text);
		range = marker->range;

		enable("ok", true);
	}else{
		enable("ok", false);
	}

	event("text", std::bind(&MarkerDialog::onEdit, this));
	event("cancel", std::bind(&MarkerDialog::onClose, this));
	event("hui:close", std::bind(&MarkerDialog::onClose, this));
	event("ok", std::bind(&MarkerDialog::onOk, this));
}

MarkerDialog::~MarkerDialog()
{
}

void MarkerDialog::onEdit()
{
	enable("ok", getString("text").num > 0);
}

void MarkerDialog::onOk()
{
	if (marker){
		track->edit_marker(marker, marker->range, getString("text"));
	}else{
		track->add_marker(range, getString("text"));
	}
	destroy();
}

void MarkerDialog::onClose()
{
	destroy();
}
