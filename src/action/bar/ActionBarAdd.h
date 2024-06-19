/*
 * ActionBarAdd.h
 *
 *  Created on: 15.12.2012
 *      Author: michi
 */

#ifndef ACTIONSONGBARADD_H_
#define ACTIONSONGBARADD_H_

#include "../ActionGroup.h"

namespace tsunami {

class BarPattern;
class Bar;
enum class BarEditMode;

class ActionBarAdd : public ActionGroup {
public:
	ActionBarAdd(int index, const BarPattern &pattern, BarEditMode mode);

	void build(Data *d) override;

	int index;
	Bar *bar;
	BarEditMode mode;
};

}

#endif /* ACTIONSONGBARADD_H_ */
