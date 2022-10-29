/*
 * CaptureConsoleModeMulti.cpp
 *
 *  Created on: 24.09.2017
 *      Author: michi
 */

#include "CaptureConsoleModeMulti.h"
#include "CaptureTrackData.h"
#include "../CaptureConsole.h"
#include "../../audioview/AudioView.h"
#include "../../mode/ViewModeCapture.h"
#include "../../../data/base.h"
#include "../../../data/Track.h"
#include "../../../data/Song.h"
#include "../../../module/audio/PeakMeter.h"
#include "../../../module/SignalChain.h"
#include "../../../Session.h"

CaptureConsoleModeMulti::CaptureConsoleModeMulti(CaptureConsole *_cc) :
	CaptureConsoleMode(_cc)
{}

void CaptureConsoleModeMulti::enter() {
	cc->hide_control("multi_grid", false);

	// target list multi
	for (Track *t: weak(song->tracks)) {
		if ((t->type != SignalType::AUDIO) and (t->type != SignalType::MIDI))
			continue;
		if (!view->sel.has(t))
			continue;

		CaptureTrackData c;
		int i = items().num;
		c.track = t;
		c.panel = cc;
		c.id_group = "group-" + i2s(i);
		c.id_grid = "grid-" + i2s(i);
		c.id_active = "active-" + i2s(i);
		c.id_source = "source-" + i2s(i);
		c.id_peaks = "peaks-" + i2s(i);
		c.id_mapper = "mapper-" + i2s(i);
		cc->set_target("multi_grid");
		cc->add_group("!expandx\\" + t->nice_name(), 0, i, c.id_group);
		cc->set_target(c.id_group);
		cc->add_grid("!expandx", 0, 0, c.id_grid);
		cc->set_target(c.id_grid);
		cc->add_check_box("!switch", 0, 0, c.id_active);
		cc->set_tooltip(c.id_active, _("Enable recording for this track?"));

		if (t->type == SignalType::AUDIO) {
			cc->add_combo_box("", 1, 0, c.id_source);
			cc->add_button("C", 2, 0, c.id_mapper);
			cc->set_tooltip(c.id_mapper, _("Channel map..."));
		} else if (t->type == SignalType::MIDI) {
			cc->add_combo_box("", 1, 0, c.id_source);
		}
		cc->set_tooltip(c.id_source, _("Source device"));
		cc->add_drawing_area(format("!height=%d,noexpandy,hidden", PeakMeterDisplay::good_size(t->channels)), 1, 1, c.id_peaks);
		c.peak_meter_display = new PeakMeterDisplay(cc, c.id_peaks, nullptr);
		c.peak_meter_display->set_visible(false);

		items().add(c);

		event_ids.add(cc->event(c.id_source, [this] {
			on_source();
		}));
	}

	update_data_from_items();

	
	chain->start();
}

void CaptureConsoleModeMulti::allow_change_device(bool allow) {
	for (auto &c: items()) {
		c.allow_edit(allow);
	}
}

void CaptureConsoleModeMulti::on_source() {
	int index = hui::get_event()->id.sub(7)._int();
	if (index < 0 or index >= items().num)
		return;
	int n = cc->get_int("");
	auto &c = items()[index];
	c.set_device(get_source(c.track->type, n));
}

void CaptureConsoleModeMulti::leave() {
	for (auto &c: items()) {
		c.peak_meter_display->set_source(nullptr);

		delete c.peak_meter_display;
		c.peak_meter_display = nullptr;
		cc->remove_control(c.id_group);
	}
	CaptureConsoleMode::leave();
}

