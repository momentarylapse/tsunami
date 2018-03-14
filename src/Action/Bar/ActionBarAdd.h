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
	ActionBarAdd(int index, int length, int num_beats, int num_sub_beats, bool affect_data);
	~ActionBarAdd();

	virtual void build(Data *d);

	int index;
	Bar *bar;
	bool affect_data;
};

#endif /* ACTIONSONGBARADD_H_ */
