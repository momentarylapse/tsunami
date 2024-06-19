#include "SignalChainConsole.h"
#include "../signaleditor/SignalEditorTab.h"
#include "../mainview/MainView.h"
#include "../TsunamiWindow.h"
#include "../../Tsunami.h"
#include "../../Session.h"
#include "../../module/SignalChain.h"
#include "../../storage/Storage.h"

namespace tsunami {

SignalChainConsole::SignalChainConsole(Session *s, BottomBar *bar) :
		BottomBar::Console(_("Signal Chains"), "signal-chain-console", s, bar)
{
	from_resource("signal-chain-console");
	id_list = "signal-chains";

	//popup_menu = hui::create_resource_menu("popup-menu-session-manager", this);

	load_data();

	event("signal-chain-new", [this] { on_new(); });
	event("signal-chain-load", [this] { on_load(); });
	event("signal-chain-save", [this] { on_save(); });
	event("signal-chain-delete", [this] { on_delete(); });
	event(id_list, [this] { on_list_double_click(); });
	event_x(id_list, "hui:right-button-down", [this] { on_right_click(); });

	session->out_add_signal_chain >> create_data_sink<SignalChain*>([this] (SignalChain*) { load_data(); });
	session->out_remove_signal_chain >> create_data_sink<SignalChain*>([this] (SignalChain*) { load_data(); });
}

SignalChainConsole::~SignalChainConsole() {
}

string suggest_new_signal_chain_name(Session *session) {
	auto exists = [session] (const string& name) {
		for (auto c: weak(session->all_signal_chains))
			if (c->name == name)
				return true;
		return false;
	};
	if (!exists("user chain"))
		return "user chain";
	for (int n=2; n<999; n++)
		if (!exists(format("user chain %d", n)))
			return format("user chain %d", n);
	return "?";
}

void SignalChainConsole::on_new() {
	auto chain = session->create_signal_chain(suggest_new_signal_chain_name(session));
	chain->explicitly_save_for_session = true;
}

void SignalChainConsole::on_load() {
	hui::file_dialog_open(win, _("Load a signal chain"), session->storage->current_chain_directory, {"filter=*.chain", "showfilter=*.chain"}).then([this] (const Path &filename) {
		session->storage->current_chain_directory = filename.parent();
		session->load_signal_chain(filename);
	});
}

void SignalChainConsole::on_save() {
}

void SignalChainConsole::on_delete() {
}

void SignalChainConsole::on_list_double_click() {
	int n = get_int(id_list);
	if (n >= 0)
		session->main_view->open_for(session->all_signal_chains[n].get());
}

void SignalChainConsole::on_right_click() {
}

void SignalChainConsole::load_data() {
	reset(id_list);
	for (auto c: weak(session->all_signal_chains)) {
		string desc = "created by plugin";
		if (c == session->all_signal_chains[0].get())
			desc = "default signal chain - used by the program for file play back";
		else if (c->belongs_to_system)
			desc = "internal signal chain used by the program";
		else if (c->explicitly_save_for_session)
			desc = "created by user";
		add_string(id_list, format("<big>%s</big>\n      <small><span alpha=\"50%%\">%s</span></small>", c->name, desc));
	}
}

}

