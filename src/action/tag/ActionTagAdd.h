/*
 * ActionTagAdd.h
 *
 *  Created on: 12.12.2012
 *      Author: michi
 */

#ifndef ACTIONTAGADD_H_
#define ACTIONTAGADD_H_

#include "../../data/Song.h"
#include <lib/history/Action.h>

namespace tsunami {

class ActionTagAdd : public history::Action {
public:
	explicit ActionTagAdd(const Tag &tag);
	~ActionTagAdd() override;

	string name() const override { return ":##:add tag"; }

	void* execute(history::Data* d) override;
	void undo(history::Data* d) override;

private:
	Tag tag;
};

}

#endif /* ACTIONTAGADD_H_ */
