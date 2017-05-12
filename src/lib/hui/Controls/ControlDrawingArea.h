/*
 * ControlDrawingArea.h
 *
 *  Created on: 17.06.2013
 *      Author: michi
 */

#ifndef CONTROLDRAWINGAREA_H_
#define CONTROLDRAWINGAREA_H_

#include "Control.h"

namespace hui
{


class ControlDrawingArea : public Control
{
public:
	ControlDrawingArea(const string &text, const string &id);

	void hardReset();

	void *cur_cairo;
};

};

#endif /* CONTROLDRAWINGAREA_H_ */
