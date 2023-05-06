/*
 * BottomBarExpandButton.h
 *
 *  Created on: 7 May 2023
 *      Author: michi
 */

#ifndef SRC_VIEW_AUDIOVIEW_GRAPH_BOTTOMBAREXPANDBUTTON_H_
#define SRC_VIEW_AUDIOVIEW_GRAPH_BOTTOMBAREXPANDBUTTON_H_

#include "../../helper/graph/Node.h"

class AudioView;
class BottomBar;

class BottomBarExpandButton : public scenegraph::Node {
public:
	AudioView *view;
	BottomBarExpandButton(AudioView *_view);
	BottomBar *bottom_bar() const;
	void on_draw(Painter *p) override;
	bool on_left_button_down(const vec2 &m) override;
	string get_tip() const override;
};



#endif /* SRC_VIEW_AUDIOVIEW_GRAPH_BOTTOMBAREXPANDBUTTON_H_ */
