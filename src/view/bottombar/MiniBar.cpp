/*
 * MiniBar.cpp
 *
 *  Created on: 23.03.2014
 *      Author: michi
 */

#include "MiniBar.h"
#include "BottomBar.h"
#include "../audioview/AudioView.h"
#include "../helper/PeakMeterDisplay.h"
#include "../../Session.h"
#include "../../device/DeviceManager.h"
#include "../../device/stream/AudioOutput.h"

MiniBar::MiniBar(BottomBar *_bottom_bar, Session *_session) {
	session = _session;
	view = session->view;
	dev_manager = session->device_manager;
	bottom_bar = _bottom_bar;

	from_resource("mini_bar");

	peak_meter = new PeakMeterDisplay(this, "peaks", view->peak_meter);
	set_float("volume", dev_manager->get_output_volume());

	on_selection_snap_mode(view->selection_snap_mode);

	event("show_bottom_bar", [=]{ on_show_bottom_bar(); });
	event("volume", [=]{ on_volume(); });
	event("select-snap-mode-free", [=]{ on_selection_snap_mode(SelectionSnapMode::NONE); });
	event("select-snap-mode-bars", [=]{ on_selection_snap_mode(SelectionSnapMode::BAR); });
	event("select-snap-mode-parts", [=]{ on_selection_snap_mode(SelectionSnapMode::PART); });

	bottom_bar->subscribe(this, [=]{ on_bottom_bar_update(); }, bottom_bar->MESSAGE_ANY);
	dev_manager->subscribe(this, [=]{ on_volume_change(); }, dev_manager->MESSAGE_ANY);
	view->subscribe(this, [=]{ on_view_settings_change(); }, view->MESSAGE_SETTINGS_CHANGE);
}

MiniBar::~MiniBar() {
	view->unsubscribe(this);
	dev_manager->unsubscribe(this);
	bottom_bar->unsubscribe(this);
}

void MiniBar::on_show_bottom_bar() {
	bottom_bar->_show();
	hide();
}

void MiniBar::on_volume() {
	dev_manager->set_output_volume(get_float(""));
}

void MiniBar::on_selection_snap_mode(SelectionSnapMode mode) {
	view->set_selection_snap_mode(mode);
	check("select-snap-mode-free", mode == SelectionSnapMode::NONE);
	check("select-snap-mode-bars", mode == SelectionSnapMode::BAR);
	check("select-snap-mode-parts", mode == SelectionSnapMode::PART);
}

void MiniBar::on_show() {
	peak_meter->enable(true);
}

void MiniBar::on_hide() {
	peak_meter->enable(false);
}

void MiniBar::on_bottom_bar_update() {
	if (bottom_bar->visible)
		hide();
	else
		show();
}

void MiniBar::on_volume_change() {
	set_float("volume", dev_manager->get_output_volume());
}


void MiniBar::on_view_settings_change() {
	set_int("selection_snap_mode", (int)view->selection_snap_mode);
}
