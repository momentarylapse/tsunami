/*
 * DeviceConsole.cpp
 *
 *  Created on: 12.04.2016
 *      Author: michi
 */

#include "DeviceConsole.h"
#include "../../Session.h"
#include "../../device/Device.h"
#include "../../device/DeviceManager.h"

static string list_id(DeviceType type) {
	if (type == DeviceType::AUDIO_OUTPUT)
		return "output-list";
	if (type == DeviceType::AUDIO_INPUT)
		return "input-list";
	if (type == DeviceType::MIDI_INPUT)
		return "midi-input-list";
	return "?";
}

static const Array<DeviceType> LIST_TYPE = {DeviceType::AUDIO_OUTPUT, DeviceType::AUDIO_INPUT, DeviceType::MIDI_INPUT};

DeviceConsole::DeviceConsole(Session *session, BottomBar *bar) :
	BottomBar::Console(_("Devices"), "device-console", session, bar)
{
	device_manager = session->device_manager;
	popup_device = nullptr;


	from_resource("device-manager");

	for (auto t: LIST_TYPE) {
		event_x(list_id(t), "hui:move", [this, t] { on_move_device(t); });
		event_x(list_id(t), "hui:right-button-down", [this, t] { on_right_click_device(t); });
	}
	event("device-delete", [this] { on_device_erase(); });
	event("device-hide", [this] { on_device_hide(); });
	event("device-sort-up", [this] { on_device_sort(SortMode::UP); });
	event("device-sort-down", [this] { on_device_sort(SortMode::DOWN); });
	event("device-sort-top", [this] { on_device_sort(SortMode::TOP); });
	event("device-sort-bottom", [this] { on_device_sort(SortMode::BOTTOM); });

	popup = hui::create_resource_menu("popup-menu-device", this);

	device_manager->out_add_device >> create_sink([this] {
		on_add_device();
	});
	device_manager->out_remove_device >> create_sink([this] {
		update_full();
	});
	device_manager->out_changed >> create_sink([this] {
		change_data();
	});

	update_full();
}

DeviceConsole::~DeviceConsole() {
	device_manager->unsubscribe(this);
	delete popup;
}

void DeviceConsole::on_device_hide() {
	popup_device->visible = !popup_device->visible;
	device_manager->set_device_config(popup_device);
}

string shorten(const string &s, int max_length) {
	if (s.num <= max_length)
		return s;
	return s.head(max_length/2-1) + "\u2026" + s.tail(max_length/2-1);
}

string DeviceConsole::to_format(const Device *d) {
	string pre, post;
	if (!d->visible or !d->present) {
		pre = "<i><span alpha=\"50%\">";
		post = "</span></i>";
	} else if (d == session->device_manager->choose_device(d->type)) {
		pre = "<b>";
		post = "</b>";
	}
	string status_icon = pre + (d->present ? "<big>✔</big>" : "<big>✘</big>") + post;
	string index = i2s(fav_index(d) + 1) + "   ";
	if (!d->visible)
		index = "";
	index = pre + index + post;
	string name = pre + d->get_name() + post;
	Array<string> infos;
	if (d == session->device_manager->choose_device(d->type))
		infos.add("preferred");
	if (!d->present)
		infos.add("missing");
	if (!d->visible)
		infos.add("ignored");
	if (d->type == DeviceType::AUDIO_OUTPUT or d->type == DeviceType::AUDIO_INPUT)
		infos.add(format("%d channels", d->channels));
	string info = pre + implode(infos, ", ") + post;
	return format("%s\\%s\\%s\n<small>    %s</small>", index, status_icon, name, info);
}

int DeviceConsole::fav_index(const Device *d) {
	int i = 0;
	for (auto *dd: device_manager->device_list(d->type)) {
		if (d == dd)
			return i;
		if (dd->visible)
			i ++;
	}
	return -1;
}

void DeviceConsole::on_right_click_device(DeviceType type) {
	int n = hui::get_event()->row;
	if (n >= 0) {
		popup_device = device_manager->device_list(type)[n];
		popup->enable("device-delete", !popup_device->present);
		popup->check("device-hide", !popup_device->visible);
		popup->open_popup(this);
	}
}

void DeviceConsole::change_data() {
	for (auto t: LIST_TYPE)
		foreachi(Device *d, device_manager->device_list(t), i)
			change_string(list_id(t), i, to_format(d));
}

void DeviceConsole::update_full() {
	for (auto t: LIST_TYPE) {
		Device *sel = nullptr;
		auto &devs = device_manager->device_list(t);
		auto id = list_id(t);
		int index = get_int(id);
		if (index >= 0)
			sel = devs[index];

		reset(id);
		foreachi(Device *d, devs, i) {
			add_string(id, to_format(d));
			if (d == sel)
				set_int(id, i);
		}
	}
}

void DeviceConsole::on_add_device() {
	auto type = device_manager->msg_type;
	auto d = device_manager->device_list(type).back();
	add_string(list_id(type), to_format(d));
}

void DeviceConsole::on_move_device(DeviceType type) {
	auto e = hui::get_event();
	int source = e->row;
	int target = e->row_target;
	//msg_write(format("  move  %d  ->  %d", source, target));
	device_manager->move_device_priority(device_manager->device_list(type)[source], target);
	set_int(list_id(type), target);
}

void DeviceConsole::on_device_sort(SortMode mode) {
	int t = get_int("dev-tab");
	auto type = LIST_TYPE[t];
	int n = get_int(list_id(type));
	if (n < 0)
		return;

	auto& list = device_manager->device_list(type);
	int target = n;
	if (mode == SortMode::UP)
		target = max(n - 1, 0);
	if (mode == SortMode::DOWN)
		target = min(n + 1, list.num - 1);
	if (mode == SortMode::TOP)
		target = 0;
	if (mode == SortMode::BOTTOM)
		target = list.num - 1;
	device_manager->move_device_priority(list[n], target);

	set_int(list_id(type), target);
}

void DeviceConsole::on_device_erase() {
	int t = get_int("dev-tab");
	auto type = LIST_TYPE[t];
	int n = get_int(list_id(type));
	if (n >= 0)
		device_manager->remove_device(type, n);
}
