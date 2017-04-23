/*
 * ActionBarAdd.h
 *
 *  Created on: 15.12.2012
 *      Author: michi
 */

#ifndef ACTIONSONGBARADD_H_
#define ACTIONSONGBARADD_H_

#include "../ActionGroup.h"

#include "../../Data/Rhythm.h"

class Song;

class ActionBarAdd : public ActionGroup
{
public:
	ActionBarAdd(int index, BarPattern &bar, bool affect_data);

	virtual void build(Data *d);

	int index;
	BarPattern bar;
	bool affect_data;
};

#endif /* ACTIONSONGBARADD_H_ */
