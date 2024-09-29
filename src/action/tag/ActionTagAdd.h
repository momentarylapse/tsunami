/*
 * ActionTagAdd.h
 *
 *  Created on: 12.12.2012
 *      Author: michi
 */

#ifndef ACTIONTAGADD_H_
#define ACTIONTAGADD_H_

#include "../../data/Song.h"
#include "../Action.h"

namespace tsunami {

class ActionTagAdd : public Action
{
public:
	explicit ActionTagAdd(const Tag &tag);
	~ActionTagAdd() override;

	string name() const override { return ":##:add tag"; }

	void *execute(Data *d) override;
	void undo(Data *d) override;

private:
	Tag tag;
};

}

#endif /* ACTIONTAGADD_H_ */
