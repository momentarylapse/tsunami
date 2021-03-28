/*
 * CaptureConsoleModeMulti.cpp
 *
 *  Created on: 24.09.2017
 *      Author: michi
 */

#include "CaptureConsoleModeMulti.h"
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
	sources_audio = session->device_manager->good_device_list(DeviceType::AUDIO_INPUT);
	sources_midi = session->device_manager->good_device_list(DeviceType::MIDI_INPUT);

	// target list multi
	for (Track *t: weak(song->tracks)) {
		if ((t->type != SignalType::AUDIO) and (t->type != SignalType::MIDI))
			continue;
		if (!view->sel.has(t))
			continue;
		CaptureItem c;
		int i = items.num;
		c.device = nullptr;
		c.track = t;
		c.enabled = false;
		c.input_audio = nullptr;
		c.input_midi = nullptr;
		c.id_group = "group-" + i2s(i);
		c.id_grid = "grid-" + i2s(i);
		c.id_target = "target-" + i2s(i);
		c.id_active = "active-" + i2s(i);
		c.id_source = "source-" + i2s(i);
		c.id_peaks = "peaks-" + i2s(i);
		c.id_mapper = "mapper-" + i2s(i);
		cc->set_target("multi_grid");
		cc->add_group("!expandx\\" + t->nice_name(), 0, i, c.id_group);
		cc->set_target(c.id_group);
		cc->add_grid("!expandx", 0, 0, c.id_grid);
		cc->set_target(c.id_grid);
		//cc->add_label(_("Source"), 0, 0, "");
		cc->add_check_box("!switch", 0, 0, c.id_active);
		cc->set_tooltip(c.id_active, _("Enable recording for this track?"));

		//cc->add_label(signal_type_name(t->type), 1, i*2+1, c.id_type);
		if (t->type == SignalType::AUDIO) {
			//c.device = session->device_manager->choose_device(DeviceType::AUDIO_INPUT);

			//c.input_audio->set_backup_mode(BACKUP_MODE_TEMP); TODO
			cc->add_combo_box(_("        - none -"), 1, 0, c.id_source);
			for (Device *d: sources_audio)
				cc->add_string(c.id_source, d->get_name());
			cc->add_button("C", 2, 0, c.id_mapper);
			cc->set_tooltip(c.id_mapper, _("Channel map..."));
		} else if (t->type == SignalType::MIDI) {
			//c.device = session->device_manager->choose_device(DeviceType::MIDI_INPUT);

			cc->add_combo_box(_("        - none -"), 2, i*2+1, c.id_source);
			for (Device *d: sources_midi)
				cc->add_string(c.id_source, d->get_name());
		}
		cc->set_tooltip(c.id_source, _("Source device"));
		cc->add_drawing_area(format("!height=%d,noexpandy,hidden", PeakMeterDisplay::good_size(t->channels)), 1, 1, c.id_peaks);
		c.peak_meter_display = new PeakMeterDisplay(cc, c.id_peaks, nullptr);
		c.peak_meter_display->set_visible(false);

		items.add(c);
		cc->event(c.id_source, [=]{ on_source(); });
	}

	for (auto &c: items) {
		if (c.track->type == SignalType::AUDIO) {
			cc->event(c.id_mapper, [&] {
				if (!c.device)
					return;
				auto map = c.channel_map;
				auto dlg = new ChannelMapDialog(cc, c.device->channels, c.track->channels, map, c.peak_meter);
				dlg->run();
				delete dlg;
				c.set_map(map);
			});
		}
		cc->event(c.id_active, [&] {
			c.enable(cc->is_checked(""));
		});
	}

	update_data_from_items();
	
	chain->start();
}

void CaptureConsoleModeMulti::allow_change_device(bool allow) {
	for (auto &c: items) {
		cc->enable(c.id_source, allow);
		cc->enable(c.id_mapper, allow);
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
	c.set_device(get_source(c.track->type, n - 1), chain.get());
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


