/*
 * ActionBarAdd.h
 *
 *  Created on: 15.12.2012
 *      Author: michi
 */

#ifndef ACTIONSONGBARADD_H_
#define ACTIONSONGBARADD_H_

#include "../ActionGroup.h"

class BarPattern;
class Bar;

class ActionBarAdd : public ActionGroup {
public:
	ActionBarAdd(int index, const BarPattern &pattern, int mode);

	void build(Data *d) override;

	int index;
	Bar *bar;
	int mode;
};

#endif /* ACTIONSONGBARADD_H_ */
