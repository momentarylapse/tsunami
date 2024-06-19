/*
 * ActionBarEdit.h
 *
 *  Created on: 15.12.2012
 *      Author: michi
 */

#ifndef ACTIONBAREDIT_H_
#define ACTIONBAREDIT_H_

#include "../ActionGroup.h"

namespace tsunami {

class Song;
class BarPattern;
enum class BarEditMode;

class ActionBarEdit: public ActionGroup {
public:
	ActionBarEdit(int index, const BarPattern &bar, BarEditMode mode);

	void build(Data *d) override;

	int index;
	int length, divisor;
	Array<int> beats;
	BarEditMode mode;
};

}

#endif /* ACTIONBAREDIT_H_ */
