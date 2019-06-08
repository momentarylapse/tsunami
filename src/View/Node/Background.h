/*
 * Background.h
 *
 *  Created on: 08.06.2019
 *      Author: michi
 */

#ifndef SRC_VIEW_NODE_BACKGROUND_H_
#define SRC_VIEW_NODE_BACKGROUND_H_

#include "ViewNode.h"

class Background : public ViewNode {
public:
	Background(AudioView *view);

	bool on_left_button_down() override;
	bool on_right_button_down() override;

	void draw(Painter *p) override;
};

#endif /* SRC_VIEW_NODE_BACKGROUND_H_ */
