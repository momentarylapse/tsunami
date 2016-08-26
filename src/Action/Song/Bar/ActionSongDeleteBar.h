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
	ActionSongDeleteBar(Song *s, int index, bool affect_midi);
};

#endif /* ACTIONSONGDELETEBAR_H_ */
