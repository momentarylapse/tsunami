/*
 * ChannelMapperDialog.cpp
 *
 *  Created on: Mar 28, 2021
 *      Author: michi
 */

#include "ChannelMapperDialog.h"
#include "../Helper/PeakMeterDisplay.h"
#include "../../Module/Audio/AudioChannelSelector.h"

string i2s_small(int i);


ChannelMapDialog::ChannelMapDialog(hui::Panel *parent, AudioChannelSelector *sel) :
		hui::Dialog("Channel map", 400, 400, parent->win, false),
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
