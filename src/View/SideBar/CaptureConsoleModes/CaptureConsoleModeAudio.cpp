/*
 * CaptureConsoleModeAudio.cpp
 *
 *  Created on: 24.09.2017
 *      Author: michi
 */

#include "CaptureConsoleModeAudio.h"
#include "CaptureTrackData.h"
#include "../CaptureConsole.h"
#include "../../../Data/Song.h"
#include "../../../Data/Track.h"
#include "../../../Data/base.h"
#include "../../AudioView.h"
#include "../../Mode/ViewModeCapture.h"
#include "../../../Session.h"
#include "../../../Module/SignalChain.h"


#include "../../../Device/Device.h"
#include "../../../Module/Audio/AudioChannelSelector.h"
#include "../../../Device/DeviceManager.h"


CaptureConsoleModeAudio::CaptureConsoleModeAudio(CaptureConsole *_cc) :
		CaptureConsoleMode(_cc) {
}

void CaptureConsoleModeAudio::on_source() {
	int n = cc->get_int("");
	items[0].set_device(get_source(SignalType::AUDIO, n));
}

void CaptureConsoleModeAudio::set_target(Track *t) {
	items[0].track = t;

	bool ok = (items[0].track->type == SignalType::AUDIO);
	cc->set_string("message", "");
	if (!ok)
		cc->set_string("message", format(_("Please select a track of type %s."), signal_type_name(SignalType::AUDIO).c_str()));
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
		items.add(a);
	}

	for (Track *t: weak(view->song->tracks))
		if (view->sel.has(t) and (t->type == SignalType::AUDIO))
			set_target(t);

	chain = session->create_signal_chain_system("capture");


	update_data_from_items();

	event_ids.add(cc->event("source", [=]{ on_source(); }));


	auto &c = items[0];
	c.enable(true);

	chain->start(); // for preview
}

void CaptureConsoleModeAudio::allow_change_device(bool allow) {
	items[0].allow_edit(allow);
}
