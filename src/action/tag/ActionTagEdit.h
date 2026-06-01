/*
 * ActionTagEdit.h
 *
 *  Created on: 12.12.2012
 *      Author: michi
 */

#ifndef ACTIONTAGDEDIT_H_
#define ACTIONTAGDEDIT_H_

#include "../../data/Song.h"
#include <lib/history/MergableAction.h>

namespace tsunami {

class ActionTagEdit : public history::MergableValueAction<string> {
public:
	ActionTagEdit(int index, const Tag& tag);

	string name() const override { return ":##:edit tag"; }

	void* execute(history::Data* d) override;
	void undo(history::Data* d) override;

	bool mergable(Action* a) override;

private:
	int index;
	Tag old_tag, new_tag;
};

}

#endif /* ACTIONTAGEDIT_H_ */
