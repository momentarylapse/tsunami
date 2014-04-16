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
class HuiWindow;

class FavoriteManager
{
public:
	FavoriteManager();
	virtual ~FavoriteManager();

	Array<string> GetList(Configurable *c);
	void Apply(Configurable *c, const string &name);
	void Save(Configurable *c, const string &name);

	string SelectName(HuiWindow *win, Configurable *c, bool save);
};

#endif /* FAVORITEMANAGER_H_ */
