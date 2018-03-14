/*
 * MarkerDialog.cpp
 *
 *  Created on: May 13, 2015
 *      Author: ankele
 */

#include "MarkerDialog.h"

MarkerDialog::MarkerDialog(hui::Window* _parent, Track* _t, const Range &_range, int _index):
	hui::Window("marker_dialog", _parent)
{
	track = _t;
	range = _range;
	index = _index;

	if (index >= 0){
		setString("text", track->markers[index]->text);
		range = track->markers[index]->range;

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
	if (index >= 0){
		track->editMarker(index, track->markers[index]->range, getString("text"));
	}else{
		track->addMarker(range, getString("text"));
	}
	destroy();
}

void MarkerDialog::onClose()
{
	destroy();
}
