/*
 * ActionSongAddBar.h
 *
 *  Created on: 15.12.2012
 *      Author: michi
 */

#ifndef ACTIONSONGADDBAR_H_
#define ACTIONSONGADDBAR_H_

#include "../../ActionGroup.h"

class Song;
class BarPattern;

class ActionSongAddBar : public ActionGroup
{
public:
	ActionSongAddBar(Song *s, int index, BarPattern &Bar, bool affect_data);
};

#endif /* ACTIONSONGADDBAR_H_ */
