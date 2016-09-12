/*
 * ActionSongDeleteLevel.h
 *
 *  Created on: 27.08.2016
 *      Author: michi
 */

#ifndef SRC_ACTION_SONG_LEVEL_ACTIONSONGDELETELEVEL_H_
#define SRC_ACTION_SONG_LEVEL_ACTIONSONGDELETELEVEL_H_

#include "../../ActionGroup.h"

class Song;

class ActionSongDeleteLevel : public ActionGroup
{
public:
	ActionSongDeleteLevel(int index);

	virtual void build(Data *d);

	int index;
};

#endif /* SRC_ACTION_SONG_LEVEL_ACTIONSONGDELETELEVEL_H_ */
