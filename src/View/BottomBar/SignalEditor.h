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
	void onMouseMove();
	void onDraw(Painter *p);

	void onChainUpdate();

	SignalChain *chain;
};

#endif /* SRC_VIEW_BOTTOMBAR_SIGNALEDITOR_H_ */
