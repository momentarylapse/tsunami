//
// Created by michi on 6/27/26.
//

#pragma once

#include <lib/base/base.h>

namespace layout {

struct Option {
	string key, value;

	string value_or(const string& default_value) const;
	string str() const;
	static Option parse(const string& s);
};

struct Resource {
	string type;
	string id;
	string title;
	Array<Option> options;
	int x = 0;
	int y = 0;
	Array<Resource> children;

	Resource* get_node(const string& id) const;
	bool enabled() const;
	bool has(const string& key) const;
	string value(const string& key, const string& fallback = "") const;
	void show(int indent = 0) const;
	string to_string(int indent = 0) const;
	string _options_str() const;

	static Resource parse(const string& buffer, bool auto_title=true, bool literal=false);
};

Resource parse_resource(const string& buffer, bool literal = false);

}
