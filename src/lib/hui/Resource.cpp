#include "hui.h"
#include "../math/math.h"
#include "Controls/Control.h"
#include "internal.h"

namespace hui
{

extern Array<Language> _languages_;
Array<Resource> _resources_;

Resource::Resource() {
	x = y = 0;
}

bool Resource::has(const string &key) {
	for (string &o: options)
		if (o == key)
			return true;
	return false;
}

bool Resource::enabled() {
	return !has("disabled");
}

string Resource::value(const string &key, const string &fallback) {
	int n = key.num + 1;
	for (string &o: options)
		if (o.head(n) == key+"=")
			return o.substr(n, -1);
	return fallback;
}

string Resource::image() {
	return value("image");
}

Resource *Resource::get_node(const string &id) const {
	for (Resource &c: children) {
		if (c.id == id)
			return &c;
		Resource *ret = c.get_node(id);
		if (ret)
			return ret;
	}
	return nullptr;
}

void LoadResourceCommand7(File *f, Resource *c) {
	c->type = f->read_str();
	c->id = f->read_str();
	if (c->id == "?")
		c->id = "id:" + i2s(randi(10000000));
	c->options = f->read_str().explode(",");
	c->x = f->read_int();
	c->y = f->read_int();
	int n = f->read_int();
	for (int i=0; i<n; i++) {
		Resource child;
		LoadResourceCommand7(f, &child);
		c->children.add(child);
	}
}

void LoadResource(const Path &filename) {
	// dirty...
	_resources_.clear();
	_languages_.clear();

	try{
		File *f = FileOpenText(filename);
		int ffv = f->ReadFileFormatVersion();
		if (ffv != 7) {
			FileClose(f);
			msg_error("hui resource version is " + i2s(ffv) + " (7 expected)");
			return;
		}

		f->read_comment();
		int nres = f->read_int();
		for (int i=0;i<nres;i++) {
			Resource res;
			res.children.clear();
			f->read_comment();
			LoadResourceCommand7(f, &res);
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
		FileClose(f);
	}catch(Exception &e) {
		msg_error(e.message());
	}
}

Resource *GetResource(const string &id) {
	for (Resource &r: _resources_)
		if (r.id == id)
			return &r;
	if (id.num > 0)
		msg_error("hui resource not found: " + id);
	return nullptr;
}

Window *CreateResourceDialog(const string &id, Window *root) {
	//return HuiCreateDialog("-dialog not found in resource-",200,100,root,true,mf);
	Resource *res = GetResource(id);
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
	Window *dlg = new Dialog(GetLanguageR(res->id, *res), width, height, root, allow_parent);

	// menu?
	if (menu_id.num > 0)
		dlg->set_menu(CreateResourceMenu(menu_id));

	// toolbar?
	if (toolbar_id.num > 0)
		dlg->toolbar[TOOLBAR_TOP]->set_by_id(toolbar_id);

	// controls
	for (Resource &cmd: res->children)
		dlg->_add_control(id, cmd, "");

	return dlg;
	
	/*msg_error(format("HuiCreateResourceDialog  (id=%d)  m(-_-)m",id));
	CHuiWindow *d=HuiCreateDialog(format("-dialog (id=%d) not found in resource-",id),300,200,root,true,mf);
	return d;*/
}

Menu *_create_res_menu_(const string &ns, Resource *res) {
	Menu *menu = new Menu();

	for (Resource &c: res->children) {
		if (c.type == "Item") {
			if (sa_contains(c.options, "checkable"))
				menu->add_checkable(get_lang(ns, c.id, c.title, true), c.id);
			else if (c.image().num > 0)
				menu->add_with_image(get_lang(ns, c.id, c.title, true), c.image(), c.id);
			else
				menu->add(get_lang(ns, c.id, c.title, true), c.id);
		} else if (c.type == "Separator") {
			menu->add_separator();
		} else if (c.type == "Menu") {
			Menu *sub = _create_res_menu_(ns, &c);
			menu->add_sub_menu(get_lang(ns, c.id, c.title, true), c.id, sub);
		}
		if (menu->items.num > 0)
			menu->items.back()->enable(c.enabled());
	}
	return menu;
}

Menu *CreateResourceMenu(const string &id) {
	Resource *res = GetResource(id);
	if (!res) {
		msg_error(format("CreateResourceMenu  (id=%s)  m(-_-)m", id));
		throw Exception(format("CreateResourceMenu  (id=%s)  m(-_-)m", id));
		return nullptr;
	}

	return _create_res_menu_(id, res);
}

Menu *CreateMenuFromSource(const string &source) {
	Resource res = ParseResource(source);

	return _create_res_menu_(res.id, &res);
}



int res_get_indent(const string &line) {
	int indent = 0;
	for (int i=0;i<line.num;i++)
		if (line[i] != '\t')
			break;
		else
			indent ++;
	return indent;
}


void res_add_option(Resource &c, const string &option) {
	if (option.head(8) == "tooltip=") {
		c.tooltip = option.substr(8, -1);
		return;
	}
	c.options.add(option);
}

bool res_load_line(const string &l, Resource &c, bool literally) {
	// parse line
	auto tokens = l.parse_tokens();
	if (tokens.num == 0)
		return false;

	c.x = 0;
	c.y = 0;

	// id
	string id;
	if (tokens.num > 1)
		id = tokens[1];
	if ((id == "?") and !literally)
		id = "rand_id:" + i2s(randi(1000000));
	if (id.head(1) == "/" and !literally)
		id = id.substr(1, -1);

	// dummy
	if (tokens[0] == ".")
		return false;
	if (tokens[0] == "Separator") {
		c.type = tokens[0];
		return true;
	}
	if (tokens.num < 3)
		return false;

	// interpret tokens
	c.type = tokens[0];
	/*if (cur_indent == 0)
		c.type = "Dialog";*/
	c.id = id;
	c.title = tokens[2];
	int n_used = 3;
	for (int i=n_used; i<tokens.num; i++)
		res_add_option(c, tokens[i]);
	return true;
}

bool res_load_rec(Array<string> &lines, int &cur_line, Resource &c, bool literally) {
	int cur_indent = res_get_indent(lines[cur_line]);
	bool r = res_load_line(lines[cur_line], c, literally);
	cur_line ++;

	if (c.type == "Grid") {

		string ind = lines[cur_line-1].head(cur_indent);

		int x = 0, y = 0;

		for (int n=0; n<1024; n++) {
			if (cur_line >= lines.num)
				break;
			int indent = res_get_indent(lines[cur_line]);
			if (indent <= cur_indent)
				break;

			if (lines[cur_line] == ind + "\t---|") {
				x = 0;
				y ++;
				cur_line ++;
				continue;
			}

			Resource child;
			if (res_load_rec(lines, cur_line, child, literally)) {
				child.x = x;
				child.y = y;
				c.children.add(child);
			}

			x ++;
		}

		return r;
	}

	for (int n=0; n<1024; n++) {
		if (cur_line >= lines.num)
			break;
		int indent = res_get_indent(lines[cur_line]);
		if (indent <= cur_indent)
			break;
		Resource child;
		if (res_load_rec(lines, cur_line, child, literally)) {
			child.x = n;
			c.children.add(child);
		}

	}
	return r;
}

void Resource::show(int indent) {
	string nn;
	for (int i=0;i<indent;i++)
		nn += "    ";
	msg_write(nn + type + " - " + id + format(" - %d %d - ", x, y) + sa2s(options));
	for (Resource &child: children)
		child.show(indent + 1);
}

string Resource::to_string(int indent) {
	string ind;
	for (int i=0;i<indent;i++)
		ind += "\t";
	string nn = ind + type;
	if (type != "Separator")
		nn += " " + id + " \"" + title.escape() + "\"";
	for (string &o: options)
		nn += " " + o;
	if (tooltip.num > 0)
		nn += " \"tooltip=" + tooltip.escape() + "\"";
	if (type == "Grid") {
		int ymax = 0;
		for (auto &c: children)
			ymax = max(ymax, c.y);
		for (int j=0; j<=ymax; j++) {
			int xmax = 0;
			for (auto &c: children)
				if (c.y == j)
					xmax = max(xmax, c.x);
			for (int i=0; i<=xmax; i++) {
				bool found = false;
				for (Resource &child: children)
					if (child.x == i and child.y == j) {
						nn += "\n" + child.to_string(indent + 1);
						found = true;
						break;
					}
				if (!found)
					nn += "\n" + ind + "\t.";
			}
			if (j < ymax)
				nn += "\n" + ind + "\t---|";
		}

	} else {
		for (Resource &child: children)
			nn += "\n" + child.to_string(indent + 1);
	}
	return nn;
}

Resource ParseResource(const string &buffer, bool literally) {
	Resource r;
	auto lines = buffer.explode("\n");
	for (int i=lines.num-1; i>=0; i--)
		if (lines[i].num == 0)
			lines.erase(i);
	int cur_line = 0;

	//HuiResourceNew c;
	res_load_rec(lines, cur_line, r, literally);
	return r;
}

};

