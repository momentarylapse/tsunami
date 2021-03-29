/*
 * CaptureConsoleModeMulti.cpp
 *
 *  Created on: 24.09.2017
 *      Author: michi
 */

#include "CaptureConsoleModeMulti.h"
#include "CaptureTrackData.h"
#include "../CaptureConsole.h"
#include "../../Dialog/ChannelMapperDialog.h"
#include "../../../Data/base.h"
#include "../../../Data/Track.h"
#include "../../../Data/Song.h"
#include "../../../Session.h"
#include "../../AudioView.h"
#include "../../Mode/ViewModeCapture.h"
#include "../../../Stuff/BackupManager.h"
#include "../../../Module/Audio/AudioSucker.h"
#include "../../../Module/Audio/PeakMeter.h"
#include "../../../Module/Audio/AudioChannelSelector.h"
//#include "../../../Module/Midi/MidiSucker.h"
#include "../../../Module/Synth/Synthesizer.h"
#include "../../../Module/ModuleFactory.h"
#include "../../../Module/SignalChain.h"
#include "../../../Device/Device.h"
#include "../../../Device/DeviceManager.h"
#include "../../../Device/Stream/AudioInput.h"
#include "../../../Device/Stream/MidiInput.h"

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
		int i = items.num;
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

		items.add(c);
		cc->event(c.id_source, [=]{ on_source(); });
	}

	update_data_from_items();

	update_device_list();

	for (auto &c: items) {
		if (c.track->type == SignalType::AUDIO) {
			cc->event(c.id_mapper, [&] {
				auto dlg = ownify(new ChannelMapDialog(cc, c.channel_selector));
				dlg->run();
			});
		}
		cc->event(c.id_active, [&] {
			c.enable(cc->is_checked(""));
		});
		c.input->subscribe(this, [=] {
			update_device_list();
		});
	}
	
	chain->start();
}

void CaptureConsoleModeMulti::allow_change_device(bool allow) {
	for (auto &c: items) {
		c.allow_edit(allow);
	}
}

Device* CaptureConsoleModeMulti::get_source(SignalType type, int i) {
	if (i >= 0) {
		if (type == SignalType::AUDIO)
			return sources_audio[i];
		if (type == SignalType::MIDI)
			return sources_midi[i];
	}
	return nullptr;
}

void CaptureConsoleModeMulti::on_source() {
	int index = hui::GetEvent()->id.substr(7, -1)._int();
	if (index < 0 or index >= items.num)
		return;
	int n = cc->get_int("");
	auto &c = items[index];
	c.set_device(get_source(c.track->type, n));
}

void CaptureConsoleModeMulti::leave() {
	for (auto c: items) {
		c.peak_meter_display->set_source(nullptr);

		delete c.peak_meter_display;
		cc->remove_control(c.id_group);
	}
	items.clear();
	session->remove_signal_chain(chain.get());
	chain = nullptr;
}



void CaptureConsoleModeMulti::update_device_list() {
	sources_audio = session->device_manager->good_device_list(DeviceType::AUDIO_INPUT);
	sources_midi = session->device_manager->good_device_list(DeviceType::MIDI_INPUT);

	for (auto &c: items) {
		auto sources = sources_audio;
		if (c.track->type == SignalType::MIDI)
			sources = sources_midi;

		// add all
		c.panel->reset(c.id_source);
		for (Device *d: sources)
			c.panel->set_string(c.id_source, d->get_name());

		// select current
		foreachi(Device *d, sources, i)
			if (d == c.get_device())
				c.panel->set_int(c.id_source, i);
	}
}
