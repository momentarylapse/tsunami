/*
 * BottomBarExpandButton.h
 *
 *  Created on: 7 May 2023
 *      Author: michi
 */

#ifndef SRC_VIEW_MAINVIEW_BOTTOMBAREXPANDBUTTON_H_
#define SRC_VIEW_MAINVIEW_BOTTOMBAREXPANDBUTTON_H_

#include "../helper/graph/Node.h"

namespace tsunami {

class Session;
class BottomBar;

class BottomBarExpandButton : public scenegraph::Node {
public:
	BottomBarExpandButton(Session *session);
	BottomBar *bottom_bar() const;
	void on_draw(Painter *p) override;
	bool on_left_button_down(const vec2 &m) override;
	string get_tip() const override;

	Session *session;
};

}

#endif /* SRC_VIEW_MAINVIEW_BOTTOMBAREXPANDBUTTON_H_ */
