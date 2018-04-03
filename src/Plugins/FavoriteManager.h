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
		int type;
		bool read_only;
	};

	bool loaded;
	Array<Favorite> favorites;

	void Load(Session *session);
	void LoadFromFile(const string &filename, bool read_only, Session *session);
	void Save(Session *session);

	void set(const Favorite &f);

	Array<string> GetList(Module *c);
	void Apply(Module *c, const string &name);
	void Save(Module *c, const string &name);

	string SelectName(hui::Window *win, Module *c, bool save);
};

#endif /* FAVORITEMANAGER_H_ */
