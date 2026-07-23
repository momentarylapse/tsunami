/*
 * ActionBarDelete.h
 *
 *  Created on: 15.12.2012
 *      Author: michi
 */

#ifndef ACTIONBARDELETE_H_
#define ACTIONBARDELETE_H_

#include <lib/history/ActionGroup.h>

namespace tsunami {

struct Song;

class ActionBarDelete: public history::ActionGroup {
public:
	ActionBarDelete(int index, bool affect_data);

	void* compose(history::Data *d) override;

	int index;
	bool affect_data;
};

}

#endif /* ACTIONBARDELETE_H_ */
