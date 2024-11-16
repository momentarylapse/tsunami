#include "DeviceSelector.h"

#include <os/msg.h>

#include "../../Session.h"
#include "../../device/Device.h"
#include "../../device/DeviceManager.h"
#include "../../lib/hui/Panel.h"

namespace tsunami {

string shorten(const string&, int);

DeviceSelector::DeviceSelector(hui::Panel* _panel, const string& _id, Session* _session, DeviceType _type) {
	panel = _panel;
	id = _id;
	session = _session;
	type = _type;

	if (panel and session) {
		update_list();
		panel->event(id, [this] {
			actively_selected = get();
			out_value(actively_selected);
		});
		session->device_manager->out_device_plugged_in >> create_data_sink<Device*>([this] (Device*) {
			update_list();
		});
		session->device_manager->out_device_plugged_out >> create_data_sink<Device*>([this] (Device*) {
			update_list();
		});
	}
}

DeviceSelector::~DeviceSelector() = default;


void DeviceSelector::__init_ext__(hui::Panel* panel, const string& id, Session* session, DeviceType type, Callable<void()> *_func) {
	new(this) DeviceSelector(panel, id, session, type);
	out_value >> create_data_sink<Device*>([_func] (Device*) { (*_func)(); });
}

void DeviceSelector::update_list() {
	list = session->device_manager->good_device_list(type);
	if (panel) {
		panel->reset(id);
		for (auto *d: list)
			panel->add_string(id, shorten(d->get_name(), 42));
		panel->set_int(id, list.find(actively_selected));
	}
}

Device* DeviceSelector::get() const {
	int n = panel->get_int(id);
	if (n >= 0)
		return list[n];
	return session->device_manager->choose_device(type);
}

void DeviceSelector::set(Device* dev) {
	actively_selected = dev;
	panel->set_int(id, list.find(actively_selected));
}

} // tsunami