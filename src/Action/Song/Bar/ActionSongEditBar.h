/*
 * ActionSongEditBar.h
 *
 *  Created on: 15.12.2012
 *      Author: michi
 */

#ifndef ACTIONSONGEDITBAR_H_
#define ACTIONSONGEDITBAR_H_

#include "../../ActionGroup.h"

class BarPattern;
class Song;

class ActionSongEditBar: public ActionGroup
{
public:
	ActionSongEditBar(int index, BarPattern &bar, bool affect_data);

	virtual void build(Data *d);

	int index;
	BarPattern &bar;
	bool affect_data;
};

#endif /* ACTIONSONGEDITBAR_H_ */
