/*
 * ActionBarEdit.h
 *
 *  Created on: 15.12.2012
 *      Author: michi
 */

#ifndef ACTIONBAREDIT_H_
#define ACTIONBAREDIT_H_

#include "../ActionGroup.h"

class BarPattern;
class Song;

class ActionBarEdit: public ActionGroup
{
public:
	ActionBarEdit(int index, BarPattern &bar, bool affect_data);

	virtual void build(Data *d);

	int index;
	BarPattern &bar;
	bool affect_data;
};

#endif /* ACTIONBAREDIT_H_ */
