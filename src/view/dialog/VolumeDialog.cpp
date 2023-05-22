/*
 * VolumeDialog.cpp
 *
 *  Created on: 22 May 2023
 *      Author: michi
 */

#include "VolumeDialog.h"
#include "../helper/VolumeControl.h"
#include "../../lib/base/pointer.h"
#include "../../data/base.h"
#include <math.h>


bool VolumeDialog::aborted;
bool VolumeDialog::maximize;



VolumeDialog::VolumeDialog(hui::Window *_parent, float value0, float min, float max, Callback _cb)
: hui::Dialog("volume-dialog", _parent) {
	cb = _cb;
	result = value0;
	aborted = true;
	maximize = false;

	volume_control = new VolumeControl(this, "slider", "value", "unit", [this] (float f) {
		result = f;
	});
	volume_control->set_range(0, 8);

	event("maximize", [this] {
		enable("value", !is_checked("maximize"));
		enable("slider", !is_checked("maximize"));
	});
	event("cancel", [this] {
		cb(-1);
		request_destroy();
	});
	event("ok", [this] {
		aborted = false;
		maximize = is_checked("maximize");
		cb(result);
		request_destroy();
	});
}

void VolumeDialog::ask(hui::Window *parent, float value0, float min, float max, Callback cb) {
	hui::fly(new VolumeDialog(parent, value0, min, max, cb));
}

