#include "hui.h"
#include "../os/file.h"
#include "../os/filesystem.h"
#include "../os/formatter.h"
#include "../os/msg.h"
#include "../math/math.h"
#include <lib/layout/Resource.h>
#include "Controls/Control.h"
#include "internal.h"

namespace hui
{

extern Array<Language> _languages_;
Array<layout::Resource> _resources_;

Array<layout::Option> parse_options(const string& s) {
	Array<layout::Option> r;
	for (const auto& x: s.explode(","))
		r.add(layout::Option::parse(x));
	return r;
}

void load_resource_command7(Stream *f, layout::Resource *c) {
	c->type = f->read_str();
	c->id = f->read_str();
	if (c->id == "?")
		c->id = "id:" + i2s(randi(10000000));
	c->options = parse_options(f->read_str());
	c->x = f->read_int();
	c->y = f->read_int();
	int n = f->read_int();
	for (int i=0; i<n; i++) {
		layout::Resource child;
		load_resource_command7(f, &child);
		c->children.add(child);
	}
}

void resource_post_process(layout::Resource &res) {
	if ((res.type == "Dialog") or (res.type == "Window")) {
		if (res.has("headerbar")) {
			if (res.children.num >= 0 and res.children[0].type == "Grid")
				if (res.children[0].children.num >= 0) {
					auto &c = res.children[0].children.back();
					if (c.type == "Grid" and c.has("buttonbar")) {
						layout::Resource hb;
						hb.id = ":header:";
						hb.type = "HeaderBar";
						hb.children = c.children;
						res.children.add(hb);
						res.children[0].children.pop();
					}
				}
		}
	}
}

void load_resource(const Path &filename) {
	// dirty...
	_resources_.clear();
	_languages_.clear();

	try{
		auto f = os::fs::open(filename, "rt");
		int ffv = f->read_str().sub_ref(1)._int();
		if (ffv != 7) {
			delete f;
			msg_error("hui resource version is " + i2s(ffv) + " (7 expected)");
			return;
		}

		f->read_comment();
		int nres = f->read_int();
		for (int i=0;i<nres;i++) {
			layout::Resource res;
			res.children.clear();
			f->read_comment();
			load_resource_command7(f, &res);
			resource_post_process(res);
			_resources_.add(res);
		}

		// languages
		f->read_comment();
		int nl = f->read_int();
		for (int l=0;l<nl;l++) {
			Language hl;

			// Language
			f->read_comment();
			hl.name = f->read_str();

			//  NumIDs
			f->read_comment();
			int n = f->read_int();
			f->read_comment(); // Text
			for (int i=0;i<n;i++) {
				Language::Command c;
				Array<string> ids = f->read_str().explode("/");
				if (ids.num >= 2)
					c._namespace = ids[0];
				if (ids.num >= 1)
					c.id = ids.back();
				c.text = f->read_str().unescape();
				c.tooltip = f->read_str().unescape();
				hl.cmd.add(c);
			}
			// Num Language Strings
			f->read_comment();
			n = f->read_int();
			// Text
			f->read_comment();
			for (int i=0;i<n;i++) {
				Language::Translation s;
				s.orig = f->read_str().unescape();
				s.trans = f->read_str().unescape();
				hl.trans.add(s);
			}
			_languages_.add(hl);
		}
		delete f;
	} catch (Exception &e) {
		msg_error(e.message());
	}
}

layout::Resource *get_resource(const string &id) {
	for (auto &r: _resources_)
		if (r.id == id)
			return &r;
	if (id.num > 0)
		msg_error("hui resource not found: " + id);
	return nullptr;
}

Window *create_resource_dialog(const string &id, Window *root) {
	//return HuiCreateDialog("-dialog not found in resource-",200,100,root,true,mf);
	auto res = get_resource(id);
	if (!res) {
		msg_error(format("CreateResourceDialog  (id=%s)  m(-_-)m", id));
		return nullptr;
	}
	

	if ((res->type != "Dialog") and (res->type != "Window")) {
		msg_error("resource type should be Dialog or Window, but is " + res->type);
		return nullptr;
	}

	string menu_id = res->value("menu");
	string toolbar_id = res->value("toolbar");
	bool allow_parent = res->has("allow-root") or res->has("allow-parent");

	// dialog
	int width = res->value("width", "300")._int();
	int height = res->value("height", "250")._int();
	Window *dlg = new Dialog(get_language_r(res->id, *res), width, height, root, allow_parent);

	// menu?
	if (menu_id.num > 0)
		dlg->set_menu(create_resource_menu(menu_id, dlg));

	// toolbar?
	if (toolbar_id.num > 0)
		if (auto t = dlg->get_toolbar(TOOLBAR_TOP))
			t->set_by_id(toolbar_id);

	// controls
	for (const auto& cmd: res->children)
		dlg->_add_control(id, cmd, "");

	return dlg;
	
	/*msg_error(format("HuiCreateResourceDialog  (id=%d)  m(-_-)m",id));
	CHuiWindow *d=HuiCreateDialog(format("-dialog (id=%d) not found in resource-",id),300,200,root,true,mf);
	return d;*/
}

xfer<Menu> _create_res_menu_(const string &ns, const layout::Resource *res, Panel *panel) {
	Menu *menu = new Menu(panel);

	for (const auto &c: res->children) {
		if (c.type == "Item") {
			if (c.has("checkable"))
				menu->add_checkable(get_lang(ns, c.id, c.title, true), c.id);
			else if (c.has("image"))
				menu->add_with_image(get_lang(ns, c.id, c.title, true), c.value("image"), c.id);
			else
				menu->add(get_lang(ns, c.id, c.title, true), c.id);
		} else if (c.type == "Separator") {
			menu->add_separator();
		} else if (c.type == "Menu") {
			Menu *sub = _create_res_menu_(ns, &c, panel);
			menu->add_sub_menu(get_lang(ns, c.id, c.title, true), c.id, sub);
		}

		if (!c.enabled())
			menu->items.back()->enable(false);
	}
	return menu;
}

xfer<Menu> create_resource_menu(const string &id, Panel *panel) {
	auto *res = get_resource(id);
	if (!res) {
		msg_error(format("CreateResourceMenu  (id=%s)  m(-_-)m", id));
		throw Exception(format("CreateResourceMenu  (id=%s)  m(-_-)m", id));
		return nullptr;
	}

	return _create_res_menu_(id, res, panel);
}

xfer<Menu> create_menu_from_source(const string &source, Panel *panel) {
	auto res = layout::parse_resource(source);

	return _create_res_menu_(res.id, &res, panel);
}

};

