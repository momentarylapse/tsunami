/*
 * ActionBarAdd.h
 *
 *  Created on: 15.12.2012
 *      Author: michi
 */

#ifndef ACTIONSONGBARADD_H_
#define ACTIONSONGBARADD_H_

#include "../ActionGroup.h"

class Bar;

class ActionBarAdd : public ActionGroup
{
public:
	ActionBarAdd(int index, int length, int num_beats, int num_sub_beats, int mode);
	~ActionBarAdd();

	virtual void build(Data *d);

	int index;
	Bar *bar;
	int mode;
};

#endif /* ACTIONSONGBARADD_H_ */
