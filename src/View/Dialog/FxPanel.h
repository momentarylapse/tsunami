/*
 * FxPanel.h
 *
 *  Created on: 20.03.2014
 *      Author: michi
 */

#ifndef FXPANEL_H_
#define FXPANEL_H_

#include "../../lib/hui/hui.h"

class Track;

class FxPanel : public HuiPanel
{
public:
	FxPanel();
	virtual ~FxPanel();

	void Clear();
	void SetTrack(Track *t);

	void OnAdd();
	void OnClose();

	string id_inner;

	Track *track;
	Array<HuiPanel*> panels;
};

#endif /* FXPANEL_H_ */
