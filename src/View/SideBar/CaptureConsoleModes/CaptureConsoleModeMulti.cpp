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

	chain = session->create_signal_chain_system("capture-multi");

	Array<CaptureTrackData> data;

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

		if (t->type == SignalType::AUDIO) {
			c.input_audio = (AudioInput*)chain->add(ModuleType::STREAM, "AudioInput");
			c.peak_meter = (PeakMeter*)chain->add(ModuleType::AUDIO_VISUALIZER, "PeakMeter");
			auto *recorder_audio = chain->add(ModuleType::PLUMBING, "AudioRecorder");
			auto *sucker = chain->add(ModuleType::PLUMBING, "AudioSucker");
			chain->connect(c.input_audio, 0, c.peak_meter, 0);
			chain->connect(c.peak_meter, 0, recorder_audio, 0);
			chain->connect(recorder_audio, 0, sucker, 0);
			data.add({c.track, c.input_audio, recorder_audio});
		} else if (t->type == SignalType::MIDI) {
			c.input_midi = (MidiInput*)chain->add(ModuleType::STREAM, "MidiInput");
			auto *recorder_midi = chain->add(ModuleType::PLUMBING, "MidiRecorder");
			auto *synth = chain->_add(t->synth->copy());
			c.peak_meter = (PeakMeter*)chain->add(ModuleType::AUDIO_VISUALIZER, "PeakMeter");
			//auto *sucker = chain->add(ModuleType::PLUMBING, "MidiSucker");
			auto *out = chain->add(ModuleType::STREAM, "AudioOutput");
			chain->connect(c.input_midi, 0, recorder_midi, 0);
			chain->connect(recorder_midi, 0, synth, 0);
			chain->connect(synth, 0, c.peak_meter, 0);
			chain->connect(c.peak_meter, 0, out, 0);
			data.add({c.track, c.input_midi, recorder_midi});
		}
		cc->add_drawing_area("!height=30,noexpandy", 2, i*2+2, c.id_peaks);
		c.peak_meter_display = new PeakMeterDisplay(cc, c.id_peaks, c.peak_meter);

		items.add(c);
		cc->event(c.id_source, [=]{ on_source(); });
	}
	chain->mark_all_modules_as_system();
	
	chain->start();
	view->mode_capture->set_data(data);
}

void CaptureConsoleModeMulti::allow_change_device(bool allow) {
	for (auto &c: items)
		cc->enable(c.id_source, allow);
}

void CaptureConsoleModeMulti::on_source() {
	int index = hui::GetEvent()->id.substr(7, -1)._int();
	if (index < 0 or index >= items.num)
		return;
	int n = cc->get_int("");
	auto &c = items[index];
	if (c.track->type == SignalType::AUDIO) {
		if (n > 0) {
			c.input_audio->set_device(sources_audio[n - 1]);
		}
	} else if (c.track->type == SignalType::MIDI) {
		if (n > 0) {
			c.input_midi->set_device(sources_midi[n - 1]);
		} else {
			c.input_midi->unconnect();
		}
	}
	/*if ((n >= 0) and (n < sources.num)) {
		chosen_device = sources[n];
		input->set_device(chosen_device);
	}*/
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


