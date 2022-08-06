/*
 * ChannelMapperDialog.cpp
 *
 *  Created on: Mar 28, 2021
 *      Author: michi
 */

#include "ChannelMapperDialog.h"
#include "../module/ConfigPanel.h"
#include "../../module/audio/AudioChannelSelector.h"


ChannelMapDialog::ChannelMapDialog(hui::Panel *parent, AudioChannelSelector *sel) :
		hui::Dialog("Channel map", 300, 100, parent->win, false) {
	set_options("", "headerbar,resizable");
	set_title("Channel map");
	add_grid("", 0, 0, "root");
	embed(sel->create_panel(), "root", 0, 0);
}
