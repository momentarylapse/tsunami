/*
 * LogNotifier.h
 *
 *  Created on: 27.03.2023
 *      Author: michi
 */

#ifndef SRC_VIEW_GRAPH_LOGNOTIFIER_H_
#define SRC_VIEW_GRAPH_LOGNOTIFIER_H_

#include "../../helper/graph/Node.h"

class AudioView;
class LogInfoBox;

class LogNotifier : public scenegraph::NodeFree {
public:
	LogNotifier(AudioView *view);
	~LogNotifier();

	bool hover(const vec2 &m) const override { return false; }

	void on_draw(Painter *p) override;

	bool progress(float dt);

	AudioView *view;


	struct Message {
		string text;
		float ttl = -1;
		float size = 0;
	} status;
	void set_status(const string &text, float size=1.0f);
	void draw_status(Painter *c, Message &m);

	LogInfoBox *info_box;
};

#endif /* SRC_VIEW_GRAPH_LOGNOTIFIER_H_ */
