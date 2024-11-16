#ifndef DEVICESELECTOR_H
#define DEVICESELECTOR_H

#include "../../lib/base/callable.h"
#include "../../lib/pattern/Observable.h"

namespace hui {
	class Panel;
}

namespace tsunami {

class Session;
class Device;
enum class DeviceType;

class DeviceSelector : public obs::Node<VirtualBase> {
public:
	DeviceSelector(hui::Panel* panel, const string& id, Session* session, DeviceType type);
	~DeviceSelector() override;

	obs::xsource<Device*> out_value{this, "value"};

	void __init_ext__(hui::Panel* panel, const string& id, Session* session, DeviceType type, Callable<void()> *_func);

	void update_list();
	Device* get() const;
	void set(Device* dev);

	hui::Panel* panel;
	string id;
	Session* session;
	DeviceType type;
	Device* actively_selected = nullptr;
	Array<Device*> list;;
};

} // tsunami

#endif //DEVICESELECTOR_H
