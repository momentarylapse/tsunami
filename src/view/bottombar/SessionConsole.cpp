/*
 * SessionConsole.cpp
 *
 *  Created on: 1 May 2022
 *      Author: michi
 */

#include "SessionConsole.h"
#include "../../lib/file/file_op.h"
#include "../../lib/xfile/xml.h"
#include "../../data/Song.h"
#include "../../plugins/TsunamiPlugin.h"
#include "../../storage/Storage.h"
#include "../../Tsunami.h"
#include "../../TsunamiWindow.h"
#include "../../Session.h"

SessionConsole::SessionConsole(Session *s, BottomBar *bar) :
	BottomBar::Console(_("Session"), "session-console", s, bar)
{
	from_resource("session-console");
	id_list = "sessions";

	load_data();

	event("save", [this] { on_save(); });
	event(id_list, [this] { on_list_double_click(); });
}

SessionConsole::~SessionConsole() {
}

Path SessionConsole::directory() const {
	return tsunami->directory << "sessions";
}

void SessionConsole::on_save() {
	dir_create(directory());
	hui::file_dialog_save(win, "", directory(), {"filter=*.session", "showfilter=*.session"}, [this] (const Path &filename) {
		if (filename)
			save_session(filename);
	});
}

void SessionConsole::on_list_double_click() {
	int n = get_int("");
	auto list = dir_search(directory(), "*.session", "f");
	if (n >= 0 and n < list.num)
		load_session(directory() << list[n]);
}

void SessionConsole::load_data() {
	auto list = dir_search(directory(), "*.session", "f");
	reset(id_list);
	for (auto &e: list)
		add_string(id_list, e.no_ext().str());

}

void SessionConsole::save_session(const Path &filename) {
	xml::Parser parser;

	auto e = xml::Element("session");
	e.add(xml::Element("head").witha("version", "1.0"));
	e.add(xml::Element("file").witha("path", session->song->filename.absolute().str()));
	auto epp = xml::Element("plugins");
	for (auto p: weak(session->plugins)) {
		auto ep = xml::Element("plugin")
				.witha("class", p->module_class)
			//	.witha("name", p->module_name)
				.witha("version", i2s(p->version()))
				.witha("config", p->config_to_string());
		epp.add(ep);
	}
	e.add(epp);
	parser.elements.add(e);
	parser.save(filename);

	load_data();
}

void SessionConsole::load_session(const Path &filename) {
	xml::Parser parser;
	parser.load(filename);
	auto &e = parser.elements[0];


	auto *s = tsunami->create_session();
	s->win->show();

	auto ef = e.find("file");

	s->storage->load(s->song.get(), ef->value("path"));


	auto epp = e.find("plugins");
	if (epp) {
		for (auto &ep: epp->elements) {
			string _class = ep.value("class");
			string config = ep.value("config");
			int version = ep.value("version")._int();

			auto p = s->execute_tsunami_plugin(_class);
			if (p and (config != "")) {
				p->config_from_string(version, config);
				p->notify();
			}
		}
	}
}

