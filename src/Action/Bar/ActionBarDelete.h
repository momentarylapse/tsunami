/*
 * ActionBarDelete.h
 *
 *  Created on: 15.12.2012
 *      Author: michi
 */

#ifndef ACTIONBARDELETE_H_
#define ACTIONBARDELETE_H_

#include "../ActionGroup.h"

class Song;

class ActionBarDelete: public ActionGroup
{
public:
	ActionBarDelete(int index, bool affect_data);

	virtual void build(Data *d);

	int index;
	bool affect_data;
};

#endif /* ACTIONBARDELETE_H_ */
