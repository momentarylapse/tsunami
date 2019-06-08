/*
 * TimeScale.h
 *
 *  Created on: 08.06.2019
 *      Author: michi
 */

#ifndef SRC_VIEW_NODE_TIMESCALE_H_
#define SRC_VIEW_NODE_TIMESCALE_H_

#include "ViewNode.h"

class TimeScale : public ViewNode {
public:
	TimeScale(AudioView *view);
	void draw(Painter *p) override;
};

#endif /* SRC_VIEW_NODE_TIMESCALE_H_ */
