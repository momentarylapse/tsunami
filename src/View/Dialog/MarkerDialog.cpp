/*
 * MarkerDialog.cpp
 *
 *  Created on: May 13, 2015
 *      Author: ankele
 */

#include "MarkerDialog.h"

MarkerDialog::MarkerDialog(HuiWindow* _parent, Track* _t, int _pos, int _index):
	HuiWindow("marker_dialog", _parent)
{
	track = _t;
	pos = _pos;
	index = _index;

	if (index >= 0){
		setString("text", track->markers[index]->text);
		pos = track->markers[index]->pos;

		enable("ok", true);
	}else{
		enable("ok", false);
	}

	event("text", this, &MarkerDialog::onEdit);
	event("cancel", this, &MarkerDialog::onClose);
	event("hui:close", this, &MarkerDialog::onClose);
	event("ok", this, &MarkerDialog::onOk);
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
		// cheap solution...
		track->song->action_manager->beginActionGroup();
		track->deleteMarker(index);
		track->addMarker(pos, getString("text"));
		track->song->action_manager->endActionGroup();
	}else{
		track->addMarker(pos, getString("text"));
	}
	destroy();
}

void MarkerDialog::onClose()
{
	destroy();
}
