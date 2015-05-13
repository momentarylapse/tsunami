/*
 * MarkerDialog.cpp
 *
 *  Created on: May 13, 2015
 *      Author: ankele
 */

#include "MarkerDialog.h"

MarkerDialog::MarkerDialog(HuiWindow* _parent, bool _allow_parent, Track* _t, int _pos):
	HuiWindow("marker_dialog", _parent, _allow_parent)
{
	track = _t;
	pos = _pos;

	enable("ok", false);

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
	track->addMarker(pos, getString("text"));
	delete(this);
}

void MarkerDialog::onClose()
{
	delete(this);
}
