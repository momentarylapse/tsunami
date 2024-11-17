/*
 * CaptureConsoleModeMidi.cpp
 *
 *  Created on: 24.09.2017
 *      Author: michi
 */

#include "CaptureConsoleModeMidi.h"
#include "CaptureTrackData.h"
#include "../CaptureConsole.h"
#include "../../audioview/AudioView.h"
#include "../../mode/ViewModeCapture.h"
#include "../../../data/base.h"
#include "../../../data/Song.h"
#include "../../../data/Track.h"
#include "../../../data/SongSelection.h"
#include "../../../module/SignalChain.h"
#include "../../../Session.h"

namespace tsunami {

CaptureConsoleModeMidi::CaptureConsoleModeMidi(CaptureConsole *_cc) :
	CaptureConsoleMode(_cc)
{}

void CaptureConsoleModeMidi::on_source() {
	const int n = console->get_int("");
	items()[0]->set_device(get_source(SignalType::Midi, n));
}


void CaptureConsoleModeMidi::enter() {
	console->hide_control("single_grid", false);

	{
		auto c = new CaptureTrackData(console, "source", "level");
		c->attach_to_gui(SignalType::Midi, session);
		add_item(c);
	}


	for (Track *t: weak(view->song->tracks))
		if (view->sel.has(t) and (t->type == SignalType::Midi))
			items()[0]->track = t;

	update_data_from_items();

	items()[0]->event_ids.add(console->event("source", [this] { on_source(); }));

	chain->set_buffer_size(512);

	auto c = items()[0];
	c->enable(true);

	chain->start();
}

void CaptureConsoleModeMidi::allow_change_device(bool allow) {
	items()[0]->allow_edit(allow);
}

}
