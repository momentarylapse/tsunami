/*
 * SessionManager.cpp
 *
 *  Created on: 1 May 2022
 *      Author: michi
 */

#include "SessionManager.h"
#include "../lib/base/base.h"
#include "../lib/file/file_op.h"
#include "../lib/xfile/xml.h"
#include "../data/Song.h"
#include "../plugins/TsunamiPlugin.h"
#include "../storage/Storage.h"
#include "../Tsunami.h"
#include "../TsunamiWindow.h"
#include "../Session.h"


/*Session *SessionManager::create(Tsunami *tsunami) {
	Session *session = new Session(tsunami->log.get(), tsunami->device_manager.get(), tsunami->plugin_manager.get(), tsunami->perf_mon.get());

	session->song = new Song(session, DEFAULT_SAMPLE_RATE);

	session->set_win(new TsunamiWindow(session));
	session->win->show();

	sessions.add(session);
	return session;
}*/

void SessionManager::save(Session *s, const Path &filename) {
	xml::Parser parser;

	auto e = xml::Element("session");
	e.add(xml::Element("head").witha("version", "1.0"));
	e.add(xml::Element("file").witha("path", s->song->filename.absolute().str()));
	auto epp = xml::Element("plugins");
	for (auto p: weak(s->plugins)) {
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
}

Session *SessionManager::load(const Path &filename) {
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

	return s;
}

Path SessionManager::directory() {
	return tsunami->directory << "sessions";
}
