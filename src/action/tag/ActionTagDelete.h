/*
 * ActionTagDelete.h
 *
 *  Created on: 12.12.2012
 *      Author: michi
 */

#ifndef ACTIONTAGDELETE_H_
#define ACTIONTAGDELETE_H_

#include "../../data/Song.h"
#include "../Action.h"

namespace tsunami {

class ActionTagDelete : public Action {
public:
	explicit ActionTagDelete(int index);
	~ActionTagDelete() override;

	string name() const override { return ":##:delete tag"; }

	void *execute(Data *d) override;
	void undo(Data *d) override;

private:
	int index;
	Tag old_tag;
};

}

#endif /* ACTIONTAGDELETE_H_ */
