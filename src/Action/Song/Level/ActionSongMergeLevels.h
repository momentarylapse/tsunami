/*
 * ActionSongMergeLevels.h
 *
 *  Created on: 27.08.2016
 *      Author: michi
 */

#ifndef SRC_ACTION_SONG_LEVEL_ACTIONSONGMERGELEVELS_H_
#define SRC_ACTION_SONG_LEVEL_ACTIONSONGMERGELEVELS_H_

#include "../../ActionGroup.h"

class Song;

class ActionSongMergeLevels : public ActionGroup
{
public:
	ActionSongMergeLevels(Song *s, int source, int target);
};

#endif /* SRC_ACTION_SONG_LEVEL_ACTIONSONGMERGELEVELS_H_ */
