/*
 * ActionBarDelete.h
 *
 *  Created on: 15.12.2012
 *      Author: michi
 */

#ifndef ACTIONBARDELETE_H_
#define ACTIONBARDELETE_H_

#include "../ActionGroup.h"

namespace tsunami {

class Song;

class ActionBarDelete: public ActionGroup {
public:
	ActionBarDelete(int index, bool affect_data);

	void build(Data *d) override;

	int index;
	bool affect_data;
};

}

#endif /* ACTIONBARDELETE_H_ */
