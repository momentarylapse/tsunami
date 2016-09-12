/*
 * ActionSongAddBar.h
 *
 *  Created on: 15.12.2012
 *      Author: michi
 */

#ifndef ACTIONSONGADDBAR_H_
#define ACTIONSONGADDBAR_H_

#include "../../ActionGroup.h"

#include "../../../Data/Rhythm.h"

class Song;

class ActionSongAddBar : public ActionGroup
{
public:
	ActionSongAddBar(int index, BarPattern &bar, bool affect_data);

	virtual void build(Data *d);

	int index;
	BarPattern bar;
	bool affect_data;
};

#endif /* ACTIONSONGADDBAR_H_ */
