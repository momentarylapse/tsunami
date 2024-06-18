/*
 * CaptureConsoleModeAudio.cpp
 *
 *  Created on: 24.09.2017
 *      Author: michi
 */

#include "CaptureConsoleModeAudio.h"
#include "CaptureTrackData.h"
#include "../CaptureConsole.h"
#include "../../audioview/AudioView.h"
#include "../../mode/ViewModeCapture.h"
#include "../../../data/Song.h"
#include "../../../data/Track.h"
#include "../../../data/base.h"
#include "../../../Session.h"
#include "../../../module/SignalChain.h"
#include "../../../device/Device.h"
#include "../../../module/audio/AudioChannelSelector.h"
#include "../../../device/DeviceManager.h"


CaptureConsoleModeAudio::CaptureConsoleModeAudio(CaptureConsole *_cc) :
		CaptureConsoleMode(_cc) {
}

void CaptureConsoleModeAudio::on_source() {
	int n = cc->get_int("");
	items()[0].set_device(get_source(SignalType::Audio, n));
}

void CaptureConsoleModeAudio::set_target(Track *t) {
	items()[0].track = t;

	bool ok = (items()[0].track->type == SignalType::Audio);
	cc->set_string("message", "");
	if (!ok)
		cc->set_string("message", format(_("Please select a track of type %s."), signal_type_name(SignalType::Audio).c_str()));
	cc->enable("start", ok);
}

void CaptureConsoleModeAudio::enter() {
	cc->hide_control("single_grid", false);

	{
		CaptureTrackData a;
		a.panel = cc;
		a.id_source = "source";
		a.id_mapper = "channel-mapper";
		a.id_peaks = "level";
		a.peak_meter_display = cc->peak_meter_display.get();
		items().add(a);
	}

	for (Track *t: weak(view->song->tracks))
		if (view->sel.has(t) and (t->type == SignalType::Audio))
			set_target(t);


	update_data_from_items();

	event_ids.add(cc->event("source", [this] { on_source(); }));


	auto &c = items()[0];
	c.enable(true);

	chain->start(); // for preview
}

void CaptureConsoleModeAudio::allow_change_device(bool allow) {
	items()[0].allow_edit(allow);
}
