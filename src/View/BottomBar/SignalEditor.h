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
class Module;
namespace hui{
	class Menu;
}
class SignalEditorTab;

class SignalEditor: public BottomBar::Console
{
public:
	SignalEditor(Session *session);
	virtual ~SignalEditor();

	void addChain(SignalChain *c);

	Array<SignalEditorTab*> tabs;

	hui::Menu *menu_chain, *menu_module;
};

#endif /* SRC_VIEW_BOTTOMBAR_SIGNALEDITOR_H_ */
