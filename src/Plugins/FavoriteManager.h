/*
 * FavoriteManager.h
 *
 *  Created on: 13.04.2014
 *      Author: michi
 */

#ifndef FAVORITEMANAGER_H_
#define FAVORITEMANAGER_H_

#include "../lib/base/base.h"

class Module;
enum class ModuleType;
namespace hui{
	class Window;
}
class Session;

class FavoriteManager
{
public:
	FavoriteManager();
	virtual ~FavoriteManager();

	static const string DEFAULT_NAME;

	struct Favorite
	{
		string name;
		string config_name;
		string options;
		ModuleType type;
		bool read_only;
	};

	bool loaded;
	Array<Favorite> favorites;

	void load(Session *session);
	void load_from_file(const string &filename, bool read_only, Session *session);
	void save(Session *session);

	void set(const Favorite &f);

	Array<string> get_list(Module *c);
	void apply(Module *c, const string &name);
	void save(Module *c, const string &name);

	string select_name(hui::Window *win, Module *c, bool save);
};

#endif /* FAVORITEMANAGER_H_ */
