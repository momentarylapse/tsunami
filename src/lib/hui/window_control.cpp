#include "Controls/Control.h"
#include "hui.h"

namespace hui {

//    for all
bool Panel::is_enabled(const string &id) {
	bool r = false;
	apply_foreach(id, [&](Control *c){ r = c->enabled; });
	return r;
}



//----------------------------------------------------------------------------------
// creating control items

Array<string> split_title(const string &title) {
	auto r = title.explode(separator);
	if (r.num > 0)
		if (r[0].head(1) == "!")
			r.erase(0);
	if (r.num == 0)
		return {""};
	return r;
}


string get_option_from_title(const string &title) {
	auto r = title.explode(separator);
	if (r.num > 0)
		if (r[0].head(1) == "!")
			return r[0].sub(1);
	return "";
}

string option_key(const string &x) {
	auto xx = x.explode("=");
	return xx[0];
}

Array<string> option_keys(const string &options) {
	auto x = options.explode(",");
	Array<string> keys;
	for (auto &y: x)
		keys.add(option_key(y));
	return keys;
}

bool option_has(const string &options, const string &key) {
	return sa_contains(option_keys(options), key);
}

string option_value(const string &options, const string &key) {
	auto r = options.explode(",");
	for (auto &x: r) {
		int p = x.find("=", 0);
		if (p >= 0)
			if (x.head(p) == key)
				return x.sub(p + 1);
	}
	return "";
}

};

