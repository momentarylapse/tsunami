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
		add_label("def", 1, 0, "l-in-def");
		for (int i=0; i<num_in; i++)
			add_label(format("in %d", i), 2+i, 0, format("l-in-%d", i));
		for (int o=0; o<num_out; o++)
			add_label(format("out %d", o), 0, 1+o, format("l-out-%d", o));

		for (int i=-1; i<num_in; i++)
			for (int o=0; o<num_out; o++) {
				string id = format("c-%d:%d", o, i);
				add_radio_button("", 2+i, 1+o, id);
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

	Array<CaptureTrackData> data;

	// target list multi
	for (Track *t: weak(song->tracks)) {
		if ((t->type != SignalType::AUDIO) and (t->type != SignalType::MIDI))
			continue;
		CaptureItem c;
		int i = items.num;
		c.input_audio = nullptr;
		c.input_midi = nullptr;
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
		add_group(t->nice_name(), 0, i, c.id_group);
		set_target(c.id_group);
		add_grid("", 0, 0, c.id_grid);
		set_target(c.id_grid);
		//add_label(t->nice_name(), 0, i*2+1, c.id_target);
		//add_label(signal_type_name(t->type), 1, i*2+1, c.id_type);
		add_label("Source", 0, 0, c.id_type);
		if (t->type == SignalType::AUDIO) {
			for (int i=0; i<t->channels; i++)
				c.channel_map.add(-1);


			//c.input_audio->set_backup_mode(BACKUP_MODE_TEMP); TODO
			add_combo_box(_("        - none -"), 1, 0, c.id_source);
			for (Device *d: sources_audio)
				add_string(c.id_source, d->get_name());
			add_button("C", 2, 0, c.id_mapper);
			set_tooltip(c.id_mapper, _("Channel map..."));
		} else if (t->type == SignalType::MIDI) {
			add_combo_box(_("        - none -"), 1, 0, c.id_source);
			for (Device *d: sources_midi)
				add_string(c.id_source, d->get_name());
		}

		if (t->type == SignalType::AUDIO) {
			c.input_audio = (AudioInput*)chain->add(ModuleType::STREAM, "AudioInput");
			c.peak_meter = (PeakMeter*)chain->add(ModuleType::AUDIO_VISUALIZER, "PeakMeter");
			auto *sucker = chain->add(ModuleType::PLUMBING, "AudioSucker");
			chain->connect(c.input_audio, 0, c.peak_meter, 0);
			chain->connect(c.peak_meter, 0, sucker, 0);
			data.add({c.track, c.input_audio, nullptr});
		} else if (t->type == SignalType::MIDI) {
			c.input_midi = (MidiInput*)chain->add(ModuleType::STREAM, "MidiInput");
			auto *synth = chain->_add(t->synth->copy());
			c.peak_meter = (PeakMeter*)chain->add(ModuleType::AUDIO_VISUALIZER, "PeakMeter");
			//auto *sucker = chain->add(ModuleType::PLUMBING, "MidiSucker");
			auto *out = chain->add(ModuleType::STREAM, "AudioOutput");
			chain->connect(c.input_midi, 0, synth, 0);
			chain->connect(synth, 0, c.peak_meter, 0);
			chain->connect(c.peak_meter, 0, out, 0);
			data.add({c.track, c.input_midi, nullptr});
		}
		add_drawing_area(format("!height=%d,noexpandy", PeakMeterDisplay::good_size(t->channels)), 1, 1, c.id_peaks);
		c.peak_meter_display = new PeakMeterDisplay(this, c.id_peaks, c.peak_meter);

		items.add(c);
		event(c.id_source, [=]{ on_source(); });
	}
	chain->mark_all_modules_as_system();

	chain->start();

	for (auto &c: items)
		if (c.track->type == SignalType::AUDIO) {
			event(c.id_mapper, [&] {
				if (!c.device)
					return;
				auto dlg = new ChannelMapDialog(this, c.device->channels, c.track->channels, c.channel_map, c.peak_meter);
				dlg->run();
				delete dlg;
				c.peak_meter_display->set_channel_map(c.channel_map);
			});
		}
}

void CaptureSetupConsole::on_leave() {
	for (auto c: items) {
		c.peak_meter_display->set_source(nullptr);

		delete c.peak_meter_display;
		remove_control(c.id_group);
	}
	items.clear();
	session->remove_signal_chain(chain.get());
	chain = nullptr;
}

void CaptureSetupConsole::on_source() {
	int index = hui::GetEvent()->id.substr(7, -1)._int();
	if (index < 0 or index >= items.num)
		return;
	int n = get_int("");
	auto &c = items[index];
	c.device = nullptr;
	if (c.track->type == SignalType::AUDIO) {
		if (n > 0) {
			c.input_audio->set_device(sources_audio[n - 1]);
			c.device = sources_audio[n - 1];
		}
	} else if (c.track->type == SignalType::MIDI) {
		if (n > 0) {
			c.input_midi->set_device(sources_midi[n - 1]);
			c.device = sources_midi[n - 1];
		} else {
			c.input_midi->unconnect();
		}
	}
	/*if ((n >= 0) and (n < sources.num)) {
		chosen_device = sources[n];
		input->set_device(chosen_device);
	}*/
}
