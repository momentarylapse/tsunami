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
#include "../../../module/SignalChain.h"
#include "../../../lib/hui/language.h"

namespace tsunami {

CaptureConsoleModeAudio::CaptureConsoleModeAudio(CaptureConsole *_cc) :
		CaptureConsoleMode(_cc) {
}

void CaptureConsoleModeAudio::on_source() {
	const int n = console->get_int("");
	items()[0].set_device(get_source(SignalType::Audio, n));
}

void CaptureConsoleModeAudio::set_target(Track *t) {
	items()[0].track = t;

	const bool ok = (items()[0].track->type == SignalType::Audio);
	console->set_string("message", "");
	if (!ok)
		console->set_string("message", format(_("Please select a track of type %s."), signal_type_name(SignalType::Audio).c_str()));
	console->enable("start", ok);
}

void CaptureConsoleModeAudio::enter() {
	console->hide_control("single_grid", false);

	{
		CaptureTrackData a;
		a.panel = console;
		a.id_source = "source";
		a.id_mapper = "channel-mapper";
		a.id_peaks = "level";
		a.peak_meter_display = console->peak_meter_display.get();
		items().add(a);
	}

	for (Track *t: weak(view->song->tracks))
		if (view->sel.has(t) and (t->type == SignalType::Audio))
			set_target(t);


	update_data_from_items();

	event_ids.add(console->event("source", [this] { on_source(); }));


	auto &c = items()[0];
	c.enable(true);

	chain->start(); // for preview
}

void CaptureConsoleModeAudio::allow_change_device(bool allow) {
	items()[0].allow_edit(allow);
}

}
