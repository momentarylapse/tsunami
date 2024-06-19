/*
 * ViewModeEditDummy.h
 *
 *  Created on: May 2, 2020
 *      Author: michi
 */

#ifndef SRC_VIEW_MODE_VIEWMODEEDITDUMMY_H_
#define SRC_VIEW_MODE_VIEWMODEEDITDUMMY_H_

#include "ViewModeDefault.h"

namespace tsunami {

class ViewModeEditDummy : public ViewModeDefault {
public:
	ViewModeEditDummy(AudioView *view);

	void on_start() override;

	string get_tip() override;
};

}

#endif /* SRC_VIEW_MODE_VIEWMODEEDITDUMMY_H_ */
