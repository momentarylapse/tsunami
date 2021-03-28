/*
 * ChannelMapperDialog.cpp
 *
 *  Created on: Mar 28, 2021
 *      Author: michi
 */

#include "ChannelMapperDialog.h"
#include "../../Module/Audio/AudioChannelSelector.h"
#include "../../Module/ConfigPanel.h"


ChannelMapDialog::ChannelMapDialog(hui::Panel *parent, AudioChannelSelector *sel) :
		hui::Dialog("Channel map", 400, 400, parent->win, false) {
	add_grid("", 0, 0, "root");
	embed(sel->create_panel(), "root", 0, 0);
}
