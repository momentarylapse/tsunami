/*
 * ActionBarAdd.h
 *
 *  Created on: 15.12.2012
 *      Author: michi
 */

#ifndef ACTIONSONGBARADD_H_
#define ACTIONSONGBARADD_H_

#include <lib/history/ActionGroup.h>

namespace tsunami {

struct BarPattern;
struct Bar;
enum class BarEditMode;

class ActionBarAdd : public history::ActionGroup {
public:
	ActionBarAdd(int index, const BarPattern &pattern, BarEditMode mode);

	void* compose(history::Data *d) override;

	int index;
	Bar *bar;
	BarEditMode mode;
};

}

#endif /* ACTIONSONGBARADD_H_ */
