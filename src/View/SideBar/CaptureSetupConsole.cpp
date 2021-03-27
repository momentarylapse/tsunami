/*
 * CaptureSetupConsole.cpp
 *
 *  Created on: Mar 27, 2021
 *      Author: michi
 */

#include "CaptureSetupConsole.h"
#include "../Mode/ViewModeCapture.h"
#include "../Helper/PeakMeterDisplay.h"
#include "../../Data/base.h"
#include "../../Data/Track.h"
#include "../../Data/Song.h"
#include "../../Session.h"
#include "../../Module/Audio/AudioSucker.h"
#include "../../Module/Audio/PeakMeter.h"
//#include "../../Module/Midi/MidiSucker.h"
#include "../../Module/Synth/Synthesizer.h"
#include "../../Module/ModuleFactory.h"
#include "../../Module/SignalChain.h"
#include "../../Device/Device.h"
#include "../../Device/DeviceManager.h"
#include "../../Device/Stream/AudioInput.h"
#include "../../Device/Stream/MidiInput.h"

string i2s_small(int i);

Array<int> create_default_channel_map(int n_in, int n_out) {
	Array<int> map;
	for (int o=0; o<n_out; o++)
		map.add(min(o, n_in-1));
	return map;
}

class ChannelMapDialog : public hui::Dialog {
public:
	int num_in, num_out;
	Array<int> &map;
	owned<PeakMeterDisplay> peak_meter_display_in;
	owned<PeakMeterDisplay> peak_meter_display_out;
	PeakMeter *peak_meter;

	ChannelMapDialog(hui::Panel *parent, int _n_in, int _n_out, Array<int> &_map, PeakMeter *pm) :
			hui::Dialog("Channel map", 400, 400, parent->win, false),
			map(_map) {
		num_in = _n_in;
		num_out = _n_out;
		peak_meter = pm;
		map.resize(num_out);

		from_resource("channel-mapper-dialog");
		set_target("grid");
		for (int i=0; i<num_in; i++)
			add_label("in" + i2s_small(i+1), 1+i, 0, format("l-in-%d", i));
		for (int o=0; o<num_out; o++)
			add_label("out" + i2s_small(o+1), 0, 1+o, format("l-out-%d", o));

		for (int i=0; i<num_in; i++)
			for (int o=0; o<num_out; o++) {
				string id = format("c-%d:%d", o, i);
				add_radio_button("", 1+i, 1+o, id);
				if (map[o] == i)
					check(id, true);
				event(id, [&] {
					auto xx = hui::GetEvent()->id.substr(2, -1).explode(":");
					int oo = xx[0]._int();
					int ii = xx[1]._int();
					//if (is_checked(id)) {
						map[oo] = ii;

						//msg_write(format(" %d %d   ", ii, oo) + ia2s(map));
						peak_meter_display_out->set_channel_map(map);
					//}
				});
			}


		peak_meter_display_in = new PeakMeterDisplay(this, "peaks-in", peak_meter);
		set_options("peaks-in", format("height=%d", PeakMeterDisplay::good_size(num_in)));
		peak_meter_display_out = new PeakMeterDisplay(this, "peaks-out", peak_meter);
		set_options("peaks-out", format("height=%d", PeakMeterDisplay::good_size(num_out)));

		peak_meter_display_out->set_channel_map(map);
	}
};


CaptureSetupConsole::CaptureSetupConsole(Session *session) :
	SideBarConsole(_("Recording setup"), session) {

	embed_dialog("recording-setup-dialog", 0, 0);

}

void CaptureSetupConsole::on_enter() {
	sources_audio = session->device_manager->good_device_list(DeviceType::AUDIO_INPUT);
	sources_midi = session->device_manager->good_device_list(DeviceType::MIDI_INPUT);

	chain = session->create_signal_chain_system("capture-multi");

//	Array<CaptureTrackData> data;

	for (Track *t: weak(song->tracks)) {
		if ((t->type != SignalType::AUDIO) and (t->type != SignalType::MIDI))
			continue;
		CaptureTrackData c;
		int i = tracks.num;
		c.track = t;
		c.device = nullptr;
		c.id_grid = "grid-" + i2s(i);
		c.id_group = "group-" + i2s(i);
		c.id_target = "target-" + i2s(i);
		c.id_type = "type-" + i2s(i);
		c.id_source = "source-" + i2s(i);
		c.id_peaks = "peaks-" + i2s(i);
		c.id_mapper = "mapper-" + i2s(i);
		set_target("mapping-grid");
		add_group("!expandx\\" + t->nice_name(), 0, i, c.id_group);
		set_target(c.id_group);
		add_grid("!expandx", 0, 0, c.id_grid);
		set_target(c.id_grid);
		//add_label(t->nice_name(), 0, i*2+1, c.id_target);
		//add_label(signal_type_name(t->type), 1, i*2+1, c.id_type);
		add_label("Source", 0, 0, c.id_type);
		c.channel_map = create_default_channel_map(t->channels, t->channels);

		if (t->type == SignalType::AUDIO) {
			add_combo_box("!expandx\\" + _("        - none -"), 1, 0, c.id_source);
			for (Device *d: sources_audio)
				add_string(c.id_source, d->get_name());
			add_button("C", 2, 0, c.id_mapper);
			set_tooltip(c.id_mapper, _("Channel map..."));
		} else if (t->type == SignalType::MIDI) {
			add_combo_box("!expandx\\" + _("        - none -"), 1, 0, c.id_source);
			for (Device *d: sources_midi)
				add_string(c.id_source, d->get_name());
		}

		add_drawing_area(format("!height=%d,noexpandy,hidden", PeakMeterDisplay::good_size(t->channels)), 1, 1, c.id_peaks);
		c.peak_meter_display = new PeakMeterDisplay(this, c.id_peaks, nullptr);

		tracks.add(c);
		event(c.id_source, [=]{ on_source(); });
	}

	rebuild_chain();
	//chain->start();

	for (auto &c: tracks)
		if (c.track->type == SignalType::AUDIO) {
			event(c.id_mapper, [&] {
				if (!c.device)
					return;
				auto dlg = new ChannelMapDialog(this, c.device->channels, c.track->channels, c.channel_map, get_input_by_device(c.device)->peak_meter);
				dlg->run();
				delete dlg;
				c.peak_meter_display->set_channel_map(c.channel_map);
			});
		}
}

void CaptureSetupConsole::on_leave() {
	for (auto c: tracks) {
		c.peak_meter_display->set_source(nullptr);

		delete c.peak_meter_display;
		remove_control(c.id_group);
	}
	tracks.clear();
	session->remove_signal_chain(chain.get());
	chain = nullptr;
}

CaptureSetupConsole::CaptureInputData *CaptureSetupConsole::get_input_by_device(Device *dev) {
	for (auto &ii: inputs)
		if (ii.device == dev)
			return &ii;
	return nullptr;
}

void CaptureSetupConsole::rebuild_chain() {

	for (auto &c: tracks) {
		c.peak_meter_display->set_source(nullptr);
	}

	chain->reset(true);
	inputs.clear();


	// build input list
	for (auto &c: tracks) {
		if (!c.device)
			continue;
		if (get_input_by_device(c.device))
			continue;
		CaptureInputData ii;
		ii.type = c.track->type;
		ii.input_audio = nullptr;
		ii.input_midi = nullptr;
		ii.device = c.device;

		if (c.track->type == SignalType::AUDIO) {
			ii.input_audio = (AudioInput*)chain->add(ModuleCategory::STREAM, "AudioInput");
			ii.input_audio->set_device(c.device);
			ii.peak_meter = (PeakMeter*)chain->add(ModuleCategory::AUDIO_VISUALIZER, "PeakMeter");
			auto *sucker = chain->add(ModuleCategory::PLUMBING, "AudioSucker");
			chain->connect(ii.input_audio, 0, ii.peak_meter, 0);
			chain->connect(ii.peak_meter, 0, sucker, 0);
		} else if (c.track->type == SignalType::MIDI) {
			ii.input_midi = (MidiInput*)chain->add(ModuleCategory::STREAM, "MidiInput");
			ii.input_midi->set_device(c.device);
			auto *synth = chain->_add(c.track->synth->copy());
			ii.peak_meter = (PeakMeter*)chain->add(ModuleCategory::AUDIO_VISUALIZER, "PeakMeter");
			auto *sucker = chain->add(ModuleCategory::PLUMBING, "AudioSucker");
			chain->connect(ii.input_midi, 0, synth, 0);
			chain->connect(synth, 0, ii.peak_meter, 0);
			chain->connect(ii.peak_meter, 0, sucker, 0);
		}
		inputs.add(ii);
	}

	// relink peak meters
	for (auto &c: tracks) {
		hide_control(c.id_peaks, !c.device);
		if (c.device) {
			c.peak_meter_display->set_source(get_input_by_device(c.device)->peak_meter);
			c.peak_meter_display->set_channel_map(c.channel_map);
		}
	}
	chain->mark_all_modules_as_system();
}

void CaptureSetupConsole::on_source() {
	int index = hui::GetEvent()->id.substr(7, -1)._int();
	if (index < 0 or index >= tracks.num)
		return;
	int n = get_int("");
	auto &c = tracks[index];
	if (n > 0) {
		if (c.track->type == SignalType::AUDIO) {
			c.device = sources_audio[n - 1];
		} else if (c.track->type == SignalType::MIDI) {
			c.device = sources_midi[n - 1];
		}
		c.channel_map = create_default_channel_map(c.device->channels, c.track->channels);
	} else {
		c.device = nullptr;
		c.channel_map = create_default_channel_map(c.track->channels, c.track->channels);
	}
	rebuild_chain();
	chain->start();
	/*if ((n >= 0) and (n < sources.num)) {
		chosen_device = sources[n];
		input->set_device(chosen_device);
	}*/
}
