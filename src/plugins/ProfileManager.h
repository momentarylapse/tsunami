/*
 * ProfileManager.h
 *
 *  Created on: 13.04.2014
 *      Author: michi
 */

#pragma once

#include "../lib/base/base.h"

class Path;
class Module;
enum class ModuleCategory;
namespace hui {
	class Window;
}
class Session;

class ProfileManager {
public:
	ProfileManager();

	static const string DEFAULT_NAME;

	struct Profile {
		ModuleCategory category;
		string _class;
		string name;
		string options;
		int version;
		bool read_only;
	};

	bool loaded;
	Array<Profile> profiles;

	void load(Session *session);
	void load_from_file(const Path &filename, bool read_only, Session *session);
	void save(Session *session);

	void set(const Profile &f);

	Array<string> get_list(Module *c);
	void apply(Module *c, const string &name, bool notify);
	void save(Module *c, const string &name);

	void select_name(hui::Window *win, Module *c, bool save, std::function<void(const string&)> cb);



	void set_favorite(Session *session, ModuleCategory type, const string &name, bool favorite);
	bool is_favorite(Session *session, ModuleCategory type, const string &name);

	struct Favorite {
		ModuleCategory type;
		string name;
		bool operator==(const Favorite&) const;
	};
	Array<Favorite> favorites;
};
