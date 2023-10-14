/*
 * ProfileManager.cpp
 *
 *  Created on: 13.04.2014
 *      Author: michi
 */

#include "../module/Module.h"
#include "../lib/base/algo.h"
#include "../lib/hui/hui.h"
#include "../lib/os/filesystem.h"
#include "../lib/doc/xml.h"
#include "../data/base.h"
#include "../view/dialog/PresetSelectionDialog.h"
#include "../Tsunami.h"
#include "../Session.h"
#include "PresetManager.h"

const string PresetManager::DEFAULT_NAME = ":def:";

PresetManager::PresetManager() {
	loaded = false;
}

void PresetManager::load_from_file_old(const Path &filename, bool read_only, Session *session) {
	if (!os::fs::exists(filename))
		return;
	try {
		xml::Parser p;
		p.load(filename);
		if (p.elements.num == 0)
			throw Exception("no root element");
		auto &root = p.elements[0];
		auto *mm = root.find("list");
		for (auto &e: mm->elements) {
			ModulePreset ff;
			string cat = e.value("category");
			ff.category = Module::category_from_str(cat);
			ff._class = e.value("class");
			ff.name = e.value("name");
			ff.version = e.value("version")._int();
			ff.options = e.text;
			ff.read_only = read_only;
			set(ff);
		}
		auto *favs = root.find("favorites");
		if (!favs)
			return;
		for (auto &e: favs->elements) {
			favorites.add({Module::category_from_str(e.value("category")), e.value("name")});
		}
	} catch (Exception &e) {
		session->e("loading profile: " + e.message());
	}
}

Array<int> parse_ints(const string& s) {
	Array<int> r;
	for (auto& x: s.sub_ref(1, -1).explode(",")) {
		r.add(x.trim()._int());
	}
	return r;
}

void PresetManager::load_from_file(const Path &filename, bool read_only, Session *session) {
	if (!os::fs::exists(filename))
		return;
	try {
		xml::Parser p;
		p.load(filename);
		if (p.elements.num == 0)
			throw Exception("no root element");
		auto &root = p.elements[0];

		if (auto *mm = root.find("modules")) {
			for (auto &e: mm->elements) {
				if (e.tag == "preset") {
					ModulePreset ff;
					ff.name = e.value("name");
					string cat = e.value("category");
					ff.category = Module::category_from_str(cat);
					ff._class = e.value("class");
					ff.version = e.value("version")._int();
					ff.options = e.text;
					ff.read_only = read_only;
					set(ff);
				} else if (e.tag == "favorite") {
					favorites.add({Module::category_from_str(e.value("category")), e.value("name")});
				}
			}
		}

		// track presets
		if (auto *tt = root.find("tracks")) {
			for (auto &e: tt->elements) {
				if (e.tag == "preset") {
					TrackPreset p;
					p.name = e.value("name");
					p.type = (SignalType)e.value("type", "0")._int();
					p.channels = e.value("channels", "1")._int();
					if (auto *i = e.find("instrument")) {
						p.instrument.type = (Instrument::Type)i->value("type")._int();
						p.instrument.string_pitch = parse_ints(i->value("strings"));
					}
					if (auto *s = e.find("synthesizer")) {
						p.synth_class = s->value("class");
						p.synth_version = s->value("version")._int();
						p.synth_options = s->text;
					}

					track_presets.add(p);
				}
			}
		}
	} catch (Exception &e) {
		session->e("loading presets: " + e.message());
	}
}

void PresetManager::make_usable(Session *session) {
	if (!loaded)
		load(session);
}

void PresetManager::load(Session *session) {
	if (os::fs::exists(tsunami->directory | "profiles.xml")) {
		// convert from legacy
		load_from_file_old(tsunami->directory | "profiles.xml", false, session);
		save(session);
		os::fs::_delete(tsunami->directory | "profiles.xml");
	} else {
		load_from_file(tsunami->directory_static | "presets-demo.xml", true, session);
		load_from_file(tsunami->directory | "presets.xml", false, session);
		save(session);
	}
	loaded = true;
}

void PresetManager::save(Session *session) {
	try {
		xml::Parser p;
		xml::Element root("presets");
		root.add_attribute("version", "1.0");

		// module presets / favorites
		xml::Element mm("modules");
		for (auto &ff: module_presets)
			if (!ff.read_only) {
				xml::Element e("preset");
				e.add_attribute("name", ff.name);
				e.add_attribute("category", Module::category_to_str(ff.category));
				e.add_attribute("class", ff._class);
				e.add_attribute("version", i2s(ff.version));
				e.text = ff.options;
				mm.add(e);
			}
		for (auto &ff: favorites) {
			xml::Element e("favorite");
			e.add_attribute("category", Module::category_to_str(ff.type));
			e.add_attribute("name", ff.name);
			mm.add(e);
		}
		root.add(mm);

		// module presets / favorites
		xml::Element tt("tracks");
		for (auto &ff: track_presets) {
			xml::Element e("preset");
			e.add_attribute("name", ff.name);
			e.add_attribute("type", i2s((int)ff.type));
			if (ff.type == SignalType::AUDIO)
				e.add_attribute("channels", i2s(ff.channels));
			if (ff.type == SignalType::MIDI) {
				xml::Element i("instrument");
				i.add_attribute("type", i2s((int)ff.instrument.type));
				i.add_attribute("strings", str(ff.instrument.string_pitch));
				e.add(i);
			}
			if (ff.type == SignalType::MIDI or ff.type == SignalType::BEATS) {
				xml::Element s("synthesizer");
				s.add_attribute("class", ff.synth_class);
				s.add_attribute("version", i2s(ff.synth_version));
				s.text = ff.synth_options;
				e.add(s);
			}
			tt.add(e);
		}
		root.add(tt);

		p.elements.add(root);
		p.save(tsunami->directory | "presets.xml");
	} catch (Exception &e) {
		session->e(e.message());
	}
}

Array<string> PresetManager::get_list(Module *c) {
	make_usable(c->session);
	Array<string> r;
	for (auto &f: module_presets) {
		if ((f.category == c->module_category) and (f._class == c->module_class))
			r.add(f.name);
	}
	return r;
}

void PresetManager::apply(Module *c, const string &name, bool notify) {
	c->reset_config();
	if (name == DEFAULT_NAME) {
		if (notify)
			c->out_changed.notify();
		return;
	}
	if (!loaded)
		load(c->session);
	for (auto &f: module_presets) {
		if ((f.category == c->module_category) and (f._class == c->module_class) and (f.name == name))
			c->config_from_string(f.version, f.options);
	}
	if (notify)
		c->out_changed.notify();
}

void PresetManager::save(Module *c, const string &name) {
	make_usable(c->session);
	ModulePreset f;
	f.category = c->module_category;
	f._class = c->module_class;
	f.name = name;
	f.version = c->version();
	f.read_only = false;
	f.options = c->config_to_string();
	set(f);
	save(c->session);
}

void PresetManager::set(const ModulePreset &ff) {
	for (auto &f: module_presets)
		if ((f.category == ff.category) and (f._class == ff._class) and (f.name == ff.name)) {
			f.options = ff.options;
			return;
		}

	module_presets.add(ff);
}

base::future<string> PresetManager::select_name(hui::Window *win, Module *c, bool save) {
	base::promise<string> promise;
	auto dlg = new PresetSelectionDialog(win, get_list(c), save);
	hui::fly(dlg).then([dlg, promise] () mutable {
		promise(dlg->selection);
	});
	return promise.get_future();
}

bool PresetManager::Favorite::operator==(const Favorite &o) const {
	return (type == o.type) and (name == o.name);
}

void PresetManager::set_favorite(Session *session, ModuleCategory type, const string &name, bool favorite) {
	make_usable(session);
	Favorite x = {type, name};
	int index = base::find_index(favorites, x);
	if (favorite) {
		if (index < 0)
			favorites.add(x);
	} else {
		if (index >= 0)
			favorites.erase(index);
	}
	save(session);
}

bool PresetManager::is_favorite(Session *session, ModuleCategory type, const string &name) {
	make_usable(session);
	Favorite x = {type, name};
	return base::find_index(favorites, x) >= 0;
}

void PresetManager::save_track_preset(Session *session, const PresetManager::TrackPreset &p) {
	make_usable(session);
	track_presets.add(p);
	save(session);
}

Array<string> PresetManager::enumerate_track_presets(Session *session) {
	make_usable(session);
	Array<string> names;
	for (const auto& p: track_presets)
		names.add(p.name);
	return names;
}

const PresetManager::TrackPreset& PresetManager::get_track_preset(Session *session, const string& name) {
	make_usable(session);
	for (const auto& p: track_presets)
		if (p.name == name)
			return p;
	return dummy_track_preset;
}
