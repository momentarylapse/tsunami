/*
 * Action__ScaleData.h
 *
 *  Created on: 26.08.2016
 *      Author: michi
 */

#ifndef SRC_ACTION_BAR_ACTION__SCALEDATA_H_
#define SRC_ACTION_BAR_ACTION__SCALEDATA_H_

#include <lib/history/Action.h>
#include "../../data/Range.h"

namespace tsunami {

class Song;

class Action__ScaleData: public history::Action {
public:
	Action__ScaleData(const Range &source, int new_size);

	virtual void* execute(history::Data* d);
	virtual void undo(history::Data* d);

	void do_scale(Song *s, const Range &r, int resize);

private:
	Range source;
	int new_size;
};

}

#endif /* SRC_ACTION_BAR_ACTION__SCALEDATA_H_ */
