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


bool VolumeDialog::maximize;



VolumeDialog::VolumeDialog(hui::Window *_parent, float value0, float min, float max)
: obs::Node<hui::Dialog>("volume-dialog", _parent) {
	result = value0;
	maximize = false;

	volume_control = new VolumeControl(this, "slider", "value", "unit");
	volume_control->out_volume >> create_data_sink<float>([this] (float f) {
		result = f;
	});
	volume_control->set_range(0, 8);

	event("maximize", [this] {
		enable("value", !is_checked("maximize"));
		enable("slider", !is_checked("maximize"));
	});
	event("cancel", [this] {
		_promise.fail();
		request_destroy();
	});
	event("ok", [this] {
		maximize = is_checked("maximize");
		_promise(result);
		request_destroy();
	});
}

base::future<float> VolumeDialog::ask(hui::Window *parent, float value0, float min, float max) {
	auto dlg = new VolumeDialog(parent, value0, min, max);
	hui::fly(dlg);
	return dlg->_promise.get_future();
}

