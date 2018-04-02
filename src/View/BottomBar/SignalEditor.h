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
namespace hui{
	class Menu;
}

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

	void onAddAudioSource();
	void onAddAudioEffect();
	void onAddAudioJoin();
	void onAddAudioInputStream();
	void onAddMidiSource();
	void onAddMidiEffect();
	void onAddSynthesizer();
	void onAddMidiInputStream();
	void onAddBeatSource();
	void onAddBeatMidifier();

	void onModuleDelete();
	void onModuleConfigure();

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

	hui::Menu *menu_chain, *menu_module;
};

#endif /* SRC_VIEW_BOTTOMBAR_SIGNALEDITOR_H_ */
