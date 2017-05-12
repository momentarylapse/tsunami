/*
 * FavoriteManager.h
 *
 *  Created on: 13.04.2014
 *      Author: michi
 */

#ifndef FAVORITEMANAGER_H_
#define FAVORITEMANAGER_H_

#include "../lib/base/base.h"

class Configurable;
namespace hui{
	class Window;
}

class FavoriteManager
{
public:
	FavoriteManager();
	virtual ~FavoriteManager();

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

	void Load();
	void LoadFromFile(const string &filename, bool read_only);
	void Save();

	void set(const Favorite &f);

	Array<string> GetList(Configurable *c);
	void Apply(Configurable *c, const string &name);
	void Save(Configurable *c, const string &name);

	string SelectName(hui::Window *win, Configurable *c, bool save);

	string type2str(int type);
	int str2type(const string &str);
};

#endif /* FAVORITEMANAGER_H_ */
