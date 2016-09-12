/*
 * ActionSongDeleteBar.h
 *
 *  Created on: 15.12.2012
 *      Author: michi
 */

#ifndef ACTIONSONGDELETEBAR_H_
#define ACTIONSONGDELETEBAR_H_

#include "../../ActionGroup.h"

class Song;

class ActionSongDeleteBar: public ActionGroup
{
public:
	ActionSongDeleteBar(int index, bool affect_data);

	virtual void build(Data *d);

	int index;
	bool affect_data;
};

#endif /* ACTIONSONGDELETEBAR_H_ */
