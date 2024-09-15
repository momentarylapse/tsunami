/*
 * AudioChannelSelector.cpp
 *
 *  Created on: Mar 28, 2021
 *      Author: michi
 */

#include "AudioChannelSelector.h"
#include "../../view/module/ConfigPanel.h"
#include "../../data/base.h"
#include "../../data/audio/AudioBuffer.h"
#include "../../stuff/BackupManager.h"
#include "../../lib/math/math.h"
#include "../../Session.h"
#include "../../plugins/PluginManager.h"
#include "../../view/helper/PeakMeterDisplay.h"


namespace kaba {
	VirtualTable* get_vtable(const VirtualBase *p);
}

namespace tsunami {

void AudioChannelSelector::Config::reset() {
	channels = 2;
	map = {0,1};
}

string AudioChannelSelector::Config::auto_conf(const string &name) const {
	if (name == "channels")
		return "1:16";
	return "";
}



AudioChannelSelector::AudioChannelSelector() : Module(ModuleCategory::Plumbing, "AudioChannelSelector") {
	peak_meter = new PeakMeter;

	auto _class = session->plugin_manager->get_class("AudioChannelSelectorConfig");
	if (_class->elements.num == 0) {
		kaba::add_class(_class);
		kaba::class_add_element("channels", kaba::TypeInt32, &Config::channels);
		_class->_vtable_location_target_ = kaba::get_vtable(&config);
	}
	config.kaba_class = _class;

	id_runner = hui::run_repeated(0.02f, [this] {
		peak_meter->out_changed.notify();
	});
}

AudioChannelSelector::~AudioChannelSelector() {
	hui::cancel_runner(id_runner);
}

ModuleConfiguration *AudioChannelSelector::get_config() const {
	return (ModuleConfiguration*)&config;
}

int AudioChannelSelector::read_audio(int port, AudioBuffer& buf) {
	if (!in.source)
		return Return::NoSource;


	AudioBuffer buf_in;
	buf_in.set_channels(config.channels);
	buf_in.resize(buf.length);
	int r = in.source->read_audio(buf_in);

	if (r > 0) {
		if (peak_meter)
			peak_meter->feed(buf_in);
		apply(buf_in.ref(0, r), buf);
	}

	return r;
}

void AudioChannelSelector::set_channel_map(int _n_in, const Array<int> &_map) {
	config.channels = _n_in;
	config.map = _map;
	changed();
}

void AudioChannelSelector::apply(const AudioBuffer &buf_in, AudioBuffer &buf_out) {
	for (int o=0; o<buf_out.channels; o++) {
		int i = min(o, buf_in.channels - 1);
		if (o < config.map.num)
			i = clamp(config.map[o], 0, buf_in.channels - 1);
		memcpy(&buf_out.c[o][0], &buf_in.c[i][0], sizeof(float) * buf_in.length);
	}
}

base::optional<int64> AudioChannelSelector::command(ModuleCommand cmd, int64 param) {
	if (cmd == ModuleCommand::SetInputChannels) {
		config.channels = (int)param;
		peak_meter->command(cmd, param);
		changed();
		return 0;
	}
	return base::None;
}


string i2s_small(int i);

class ChannelMapPanel : public ConfigPanel {
public:
	int num_in, num_out;
	Array<int> &map;
	owned<PeakMeterDisplay> peak_meter_display_in;
	owned<PeakMeterDisplay> peak_meter_display_out;
	PeakMeter *peak_meter;
	AudioChannelSelector *selector;

	explicit ChannelMapPanel(AudioChannelSelector *sel) :
			ConfigPanel(sel),
			map(sel->config.map) {
		selector = sel;
		num_in = selector->config.channels;
		num_out = map.num;
		peak_meter = selector->peak_meter.get();

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
				event(id, [&] {
					auto xx = hui::get_event()->id.sub(2).explode(":");
					int oo = xx[0]._int();
					int ii = xx[1]._int();
					map[oo] = ii;
					changed();
					peak_meter_display_out->set_channel_map(map);
				});
			}


		peak_meter_display_in = new PeakMeterDisplay(this, "peaks-in", peak_meter);
		peak_meter_display_out = new PeakMeterDisplay(this, "peaks-out", peak_meter);

		// TODO: needed?
		update();
	}

	void update() override {
		for (int i=0; i<num_in; i++)
			for (int o=0; o<num_out; o++) {
				string id = format("c-%d:%d", o, i);
				if (map[o] == i)
					check(id, true);
			}
		peak_meter_display_out->set_channel_map(map);

		set_options("peaks-in", format("height=%d", PeakMeterDisplay::good_size(num_in)));
		set_options("peaks-out", format("height=%d", PeakMeterDisplay::good_size(num_out)));
	}
};

ConfigPanel* AudioChannelSelector::create_panel() {
	return new ChannelMapPanel(this);
}

}
