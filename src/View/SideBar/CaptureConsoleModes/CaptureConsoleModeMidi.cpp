/*
 * CaptureConsoleModeMidi.cpp
 *
 *  Created on: 24.09.2017
 *      Author: michi
 */

#include "CaptureConsoleModeMidi.h"
#include "CaptureTrackData.h"
#include "../CaptureConsole.h"
#include "../../AudioView/AudioView.h"
#include "../../Mode/ViewModeCapture.h"
#include "../../../Data/base.h"
#include "../../../Data/Song.h"
#include "../../../Data/Track.h"
#include "../../../Data/SongSelection.h"
#include "../../../Module/SignalChain.h"
#include "../../../Session.h"

CaptureConsoleModeMidi::CaptureConsoleModeMidi(CaptureConsole *_cc) :
	CaptureConsoleMode(_cc)
{}

void CaptureConsoleModeMidi::on_source() {
	int n = cc->get_int("");
	items()[0].set_device(get_source(SignalType::MIDI, n));
}


void CaptureConsoleModeMidi::enter() {
	cc->hide_control("single_grid", false);

	{
		CaptureTrackData c;
		c.id_peaks = "level";
		c.id_source = "source";
		c.peak_meter_display = cc->peak_meter_display.get();
		c.panel = cc;
		items().add(c);
	}


	for (Track *t: weak(view->song->tracks))
		if (view->sel.has(t) and (t->type == SignalType::MIDI))
			items()[0].track = t;

	update_data_from_items();

	event_ids.add(cc->event("source", [=]{ on_source(); }));

	chain->set_buffer_size(512);

	auto &c = items()[0];
	c.enable(true);

	chain->start();
}

void CaptureConsoleModeMidi::allow_change_device(bool allow) {
	items()[0].allow_edit(allow);
}


