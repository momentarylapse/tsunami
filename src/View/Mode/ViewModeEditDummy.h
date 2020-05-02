/*
 * ViewModeEditDummy.h
 *
 *  Created on: May 2, 2020
 *      Author: michi
 */

#ifndef SRC_VIEW_MODE_VIEWMODEEDITDUMMY_H_
#define SRC_VIEW_MODE_VIEWMODEEDITDUMMY_H_

#include "ViewModeDefault.h"

class ViewModeEditDummy : public ViewModeDefault {
public:
	ViewModeEditDummy(AudioView *view);

	void on_start() override;
};

#endif /* SRC_VIEW_MODE_VIEWMODEEDITDUMMY_H_ */
