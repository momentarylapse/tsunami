//
// Created by michi on 6/27/26.
//

#include "Resource.h"
#include <lib/os/msg.h>
#include <lib/math/math.h>

namespace layout {

string Option::str() const {
	if (value.num > 0)
		return key + "=" + value;
	return key;
}

string Option::value_or(const string& default_value) const {
	if (value.num > 0)
		return value;
	return default_value;
}

Option Option::parse(const string& s) {
	int p = s.find("=");
	if (p > 0)
		return {s.head(p).replace("-", ""), s.sub(p + 1)};
	return {s.replace("-", ""), ""};
}

string Resource::_options_str() const {
	Array<string> r;
	for (const auto& o: options)
		r.add(str(o));
	return implode(r, ",");
}

bool Resource::has(const string& key) const {
	for (const auto& o: options)
		if (o.key == key)
			return true;
	return false;
}

bool Resource::enabled() const {
	return !has("disabled");
}

string Resource::value(const string &key, const string &fallback) const {
	for (const auto& o: options)
		if (o.key == key)
			return o.value;
	return fallback;
}

Resource *Resource::get_node(const string &id) const {
	for (Resource &c: children) {
		if (c.id == id)
			return &c;
		if (auto ret = c.get_node(id))
			return ret;
	}
	return nullptr;
}

void Resource::show(int indent) const {
	string nn = string("    ").repeat(indent);
	msg_write(nn + format("%s - %s - %d %d - %s", type, id, x, y, str(options)));
	for (const auto &child: children)
		child.show(indent + 1);
}

string Resource::to_string(int indent) const {
	const string ind = string("\t").repeat(indent);
	string nn = ind + type;
	if (type != "Separator")
		nn += " " + id + " \"" + title.escape() + "\"";
	for (const auto& o: options)
		nn += " " + str(o);
	if (has("tooltip"))
		nn += " \"tooltip=" + value("tooltip").escape() + "\"";
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
				for (const auto& child: children)
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
		for (const auto& child: children)
			nn += "\n" + child.to_string(indent + 1);
	}
	return nn;
}


int res_get_indent(const string& line) {
	int indent = 0;
	for (int i=0;i<line.num;i++)
		if (line[i] != '\t')
			break;
		else
			indent ++;
	return indent;
}

// a='b' -> 'a=b' :P
string make_string_values_parsable(const string& s) {
	string r = s;
	int pos = r.num;
	while (true) {
		int p1 = r.rfind("='", pos);
		int p2 = r.rfind("=\"", pos);
		if (p1 < 0 and p2 < 0)
			break;
		if (p1 > p2) {
			if (r[p1+2] == '\'') {
				// allow a=''
				pos = p1-1;
				continue;
			}
			int p0 = r.rfind(" ", p1);
			if (p0 < 0)
				break;
			r = r.head(p0) + " '" + r.sub_ref(p0+1, p1) + "=" + r.sub_ref(p1+2);
		} else {
			if (r[p2+2] == '\"') {
				// allow a=""
				pos = p2-1;
				continue;
			}
			int p0 = r.rfind(" ", p2);
			if (p0 < 0)
				break;
			r = r.head(p0) + " \"" + r.sub_ref(p0+1, p2) + "=" + r.sub_ref(p2+2);
		}
	}
	return r;
}

bool res_load_line(const string &l, Resource &c, bool literally, bool auto_title) {
	// parse line
	auto tokens = make_string_values_parsable(l).parse_tokens();
	if (tokens.num == 0)
		return false;

	c.x = 0;
	c.y = 0;

	int n_expected = auto_title ? 3 : 2;

	// id
	string id;
	if (tokens.num > 1)
		id = tokens[1];
	if ((id == "?") and !literally)
		id = format("rand:%06x", randi(1<<24));
	if (id.head(1) == "/" and !literally)
		id = id.sub(1);

	// dummy
	if (tokens[0] == ".")
		return false;
	if (tokens[0] == "Separator" and tokens.num == 1) {
		c.type = tokens[0];
		return true;
	}
	if (tokens.num < n_expected)
		return false;

	// interpret tokens
	c.type = tokens[0];
	c.id = id;
	if (auto_title)
		c.title = tokens[2];
	for (int i=n_expected; i<tokens.num; i++)
		c.options.add(Option::parse(tokens[i]));
	return true;
}

bool res_load_rec(Array<string> &lines, int &cur_line, Resource &c, bool literally, bool auto_title) {
	int cur_indent = res_get_indent(lines[cur_line]);
	bool r = res_load_line(lines[cur_line], c, literally, auto_title);
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
			if (res_load_rec(lines, cur_line, child, literally, auto_title)) {
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
		if (res_load_rec(lines, cur_line, child, literally, auto_title)) {
			child.x = n;
			c.children.add(child);
		}

	}
	return r;
}

Resource Resource::parse(const string &buffer, bool auto_title, bool literally) {
	Resource r;
	auto lines = buffer.explode("\n");
	for (int i=lines.num-1; i>=0; i--)
		if (lines[i].num == 0)
			lines.erase(i);
	int cur_line = 0;

	res_load_rec(lines, cur_line, r, literally, auto_title);
	return r;
}

Resource parse_resource(const string &buffer, bool literally) {
	return Resource::parse(buffer, true, literally);
}

}

