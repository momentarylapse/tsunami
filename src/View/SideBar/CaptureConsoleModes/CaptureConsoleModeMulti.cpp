/*
 * CaptureConsoleModeMulti.cpp
 *
 *  Created on: 24.09.2017
 *      Author: michi
 */

#include "CaptureConsoleModeMulti.h"
#include "../CaptureConsole.h"
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
		c.input_audio = nullptr;
		c.input_midi = nullptr;
		c.track = t;
		c.device = nullptr;
		c.id_target = "target-" + i2s(i);
		c.id_type = "type-" + i2s(i);
		c.id_source = "source-" + i2s(i);
		c.id_peaks = "peaks-" + i2s(i);
		cc->set_target("multi_grid");
		cc->add_label(t->nice_name(), 0, i*2+1, c.id_target);
		cc->add_label(signal_type_name(t->type), 1, i*2+1, c.id_type);
		if (t->type == SignalType::AUDIO) {
			//c.input_audio->set_backup_mode(BACKUP_MODE_TEMP); TODO
			cc->add_combo_box(_("        - none -"), 2, i*2+1, c.id_source);
			for (Device *d: sources_audio)
				cc->add_string(c.id_source, d->get_name());
		} else if (t->type == SignalType::MIDI) {
			cc->add_combo_box(_("        - none -"), 2, i*2+1, c.id_source);
			for (Device *d: sources_midi)
				cc->add_string(c.id_source, d->get_name());
		}
		cc->add_drawing_area(format("!height=%d,noexpandy", PeakMeterDisplay::good_size(t->channels)), 2, i*2+2, c.id_peaks);
		c.peak_meter_display = new PeakMeterDisplay(cc, c.id_peaks, nullptr);

		items.add(c);
		cc->event(c.id_source, [=]{ on_source(); });
	}

	update_data_from_items();
	
	chain->start();
}

void CaptureConsoleModeMulti::allow_change_device(bool allow) {
	for (auto &c: items)
		cc->enable(c.id_source, allow);
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
		cc->remove_control(c.id_target);
		cc->remove_control(c.id_type);
		cc->remove_control(c.id_source);
		cc->remove_control(c.id_peaks);
	}
	items.clear();
	session->remove_signal_chain(chain.get());
	chain = nullptr;
}


