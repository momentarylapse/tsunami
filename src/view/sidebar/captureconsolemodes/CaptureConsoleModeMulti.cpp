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
#include "../../../lib/hui/language.h"

namespace tsunami {

CaptureConsoleModeMulti::CaptureConsoleModeMulti(CaptureConsole *_cc) :
	CaptureConsoleMode(_cc)
{}

void CaptureConsoleModeMulti::enter() {
	console->hide_control("multi_grid", false);

	// target list multi
	for (Track *t: weak(song->tracks)) {
		if ((t->type != SignalType::Audio) and (t->type != SignalType::Midi))
			continue;
		if (!view->sel.has(t))
			continue;

		const int i = items().num + 1;
		auto c = new CaptureTrackData(console, "source-" + i2s(i), "peaks-" + i2s(i));
		c->track = t;
		c->id_group = "group-" + i2s(i);
		c->id_grid = "grid-" + i2s(i);
		c->id_active = "active-" + i2s(i);
		c->id_mapper = "mapper-" + i2s(i);
		console->set_target("multi_grid");
		console->add_group("!expandx\\" + t->nice_name(), 0, i, c->id_group);
		console->set_target(c->id_group);
		console->add_grid("!expandx", 0, 0, c->id_grid);
		console->set_target(c->id_grid);
		console->add_check_box("!switch", 0, 0, c->id_active);
		console->set_tooltip(c->id_active, _("Enable recording for this track?"));

		if (t->type == SignalType::Audio) {
			console->add_combo_box("", 1, 0, c->id_source);
			console->add_button("C", 2, 0, c->id_mapper);
			console->set_tooltip(c->id_mapper, _("Channel map..."));
		} else if (t->type == SignalType::Midi) {
			console->add_combo_box("", 1, 0, c->id_source);
		}
		console->add_drawing_area(format("!height=%d,noexpandy,hidden", PeakMeterDisplay::good_size(t->channels)), 1, 1, c->id_peaks);

		c->attach_to_gui(t->type, session);
		c->peak_meter_display->set_visible(false);

		add_item(c);
	}

	update_data_from_items();

	
	chain->start();
}

void CaptureConsoleModeMulti::allow_change_device(bool allow) {
	for (auto c: items()) {
		c->allow_edit(allow);
	}
}

void CaptureConsoleModeMulti::leave() {
	for (auto c: items()) {
		c->peak_meter_display->set_source(nullptr);
		c->peak_meter_display = nullptr;
		console->remove_control(c->id_group);
	}
	CaptureConsoleMode::leave();
}

}
