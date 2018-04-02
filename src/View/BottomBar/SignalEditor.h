/*
 * SignalEditor.h
 *
 *  Created on: 30.03.2018
 *      Author: michi
 */

#ifndef SRC_VIEW_BOTTOMBAR_SIGNALEDITOR_H_
#define SRC_VIEW_BOTTOMBAR_SIGNALEDITOR_H_


#include "BottomBar.h"

class Painter;
class SignalChain;

class SignalEditor: public BottomBar::Console
{
public:
	SignalEditor(Session *session);
	virtual ~SignalEditor();

	void onLeftButtonDown();
	void onLeftButtonUp();
	void onRightButtonDown();
	void onMouseMove();
	void onKeyDown();
	void onDraw(Painter *p);

	void onChainUpdate();

	SignalChain *chain;

	struct Selection
	{
		Selection();
		int type;
		void *module;
		int port, port_type;
		float dx, dy;
		enum{
			TYPE_MODULE,
			TYPE_PORT_IN,
			TYPE_PORT_OUT,
		};
		void *target_module;
		int target_port;
	};
	Selection getHover(float mx, float my);
	Selection hover, sel;
};

#endif /* SRC_VIEW_BOTTOMBAR_SIGNALEDITOR_H_ */
