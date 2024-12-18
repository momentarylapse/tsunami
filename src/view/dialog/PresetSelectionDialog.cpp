//
// Created by michi on 21.05.23.
//

#include "PresetSelectionDialog.h"
#include "../../lib/hui/language.h"

namespace tsunami {

PresetSelectionDialog::PresetSelectionDialog(hui::Window *parent, const Array<string> &_names, bool _save) :
		hui::Dialog("favorite-dialog", parent) {
	save = _save;
	names = _names;
	set_options("name", "placeholder=" + _("enter new name"));
	if (!save)
		add_string("list", _("-Default  Parameters-"));
	for (string &n: names)
		add_string("list", n);
	if (!save)
		names.insert(":def:", 0);
	hide_control("name", !save);
	if (save)
		set_string("ok", "Save");
	event("list", [this] { on_list(); });
	event_x("list", "hui:select", [this] { on_list_select(); });
	event("name", [this] { on_name(); });
	event("ok", [this] { on_ok(); });
	event("cancel", [this] { request_destroy(); });
}

void PresetSelectionDialog::on_list() {
	int n = get_int("list");
	selection = "";
	if (n >= 0) {
		selection = names[n];
		set_string("name", names[n]);
	}
	request_destroy();
}

void PresetSelectionDialog::on_list_select() {
	int n = get_int("list");
	if (n >= 0)
		set_string("name", names[n]);
}

void PresetSelectionDialog::on_name() {
	set_int("list", -1);
}

void PresetSelectionDialog::on_ok() {
	selection = get_string("name");
	request_destroy();
}


base::future<string> PresetSelectionDialog::ask(hui::Window* parent, const Array<string> &names, bool save) {
	base::promise<string> promise;
	auto dlg = new PresetSelectionDialog(parent, names, save);
	hui::fly(dlg).then([dlg, promise] () mutable {
		promise(dlg->selection);
	});
	return promise.get_future();
}

}
